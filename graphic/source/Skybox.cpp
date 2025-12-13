#include "Skybox.hpp"
#include "Engine.hpp"
#include "RenderManager.hpp"
#include "Texture.hpp"
#include "Shader.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include <iostream>
#include <glew.h>
#include <gtc/matrix_transform.hpp>

Skybox::Skybox() {}

Skybox::~Skybox()
{
    glDeleteTextures(1, &envCubemap);
    glDeleteTextures(1, &irradianceMap);
    glDeleteTextures(1, &prefilterMap);
    glDeleteTextures(1, &brdfLUTTexture);

    if (quadVAO != 0) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO != 0) glDeleteBuffers(1, &quadVBO);
}

void Skybox::Init(const std::string& hdrTexturePath)
{
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();

    // 1. 필요한 셰이더들 로드 (이미 로드되어 있으면 캐시된 것 사용됨)
    renderManager->LoadShader("brdf", "asset/shaders/brdf.vert", "asset/shaders/brdf.frag");
    renderManager->LoadShader("skybox_render", "asset/shaders/skybox.vert", "asset/shaders/skybox.frag");
    renderManager->LoadShader("skybox", "asset/shaders/skybox.vert", "asset/shaders/equirectangular.frag");
    renderManager->LoadShader("irradiance", "asset/shaders/skybox.vert", "asset/shaders/irradiance.frag");
    renderManager->LoadShader("prefilter", "asset/shaders/skybox.vert", "asset/shaders/prefilter.frag");

    // 2. HDR 텍스처 로드
    renderManager->LoadTexture("hdr_temp", hdrTexturePath);

    // 3. 큐브 메쉬 생성
    std::vector<Vertex> vertices = {
        {{-1.0f,  1.0f, -1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
        {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},
        {{-1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},
        {{-1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}},
        {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
        {{-1.0f, -1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}},
        {{-1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f, -1.0f}},
        {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
        {{ 1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}
    };
    std::vector<unsigned int> indices(36);
    for (int i = 0; i < 36; ++i) indices[i] = i;
    skyboxCube = std::make_shared<Mesh>(vertices, indices);
    skyboxCube->UploadToGPU();

    // 4. 사전 연산 실행
    PrecomputeIBL(hdrTexturePath);
}

void Skybox::PrecomputeIBL(const std::string& hdrPath)
{
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();
    Texture* hdrTexture = renderManager->GetTexture("hdr_temp").get();

    // IBL 생성 중에는 블렌딩이 필요 없으므로 끕니다. (이전 씬의 렌더링 상태 간섭 방지)
    glDisable(GL_BLEND);

    // 캡처용 배경색을 검은색으로 설정 (혹시 모를 색 섞임 방지)
    glm::vec4 oldClearColor;
    glGetFloatv(GL_COLOR_CLEAR_VALUE, &oldClearColor.x); // 기존 배경색 저장
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 검은색으로 설정

    // FBO, RBO 생성
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // 상태 변경 (캡처용)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    // 1. Env Cubemap 생성
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (int i = 0; i < 6; ++i) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
       glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
       glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))
    };

    std::shared_ptr<Shader> equiShader = renderManager->GetShader("skybox");
    equiShader->Bind();
    equiShader->SetUniformMat4f("projection", captureProjection);
    hdrTexture->Bind(0);
    equiShader->SetUniform1i("equirectangularMap", 0);

    glViewport(0, 0, 512, 512);
    for (int i = 0; i < 6; ++i) {
        equiShader->SetUniformMat4f("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skyboxCube->GetVertexArray()->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // 2. Irradiance Map
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (int i = 0; i < 6; ++i) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

    std::shared_ptr<Shader> irrShader = renderManager->GetShader("irradiance");
    irrShader->Bind();
    irrShader->SetUniformMat4f("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    irrShader->SetUniform1i("environmentMap", 0);

    glViewport(0, 0, 32, 32);
    for (int i = 0; i < 6; ++i) {
        irrShader->SetUniformMat4f("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        skyboxCube->GetVertexArray()->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 3. Prefilter Map
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (int i = 0; i < 6; ++i) glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    std::shared_ptr<Shader> preShader = renderManager->GetShader("prefilter");
    preShader->Bind();
    preShader->SetUniformMat4f("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    preShader->SetUniform1i("environmentMap", 0);

    unsigned int maxMipLevels = 5;
    for (unsigned int mip = 0; mip < maxMipLevels; ++mip) {
        unsigned int mipWidth = 128 * std::pow(0.5, mip);
        unsigned int mipHeight = 128 * std::pow(0.5, mip);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        preShader->SetUniform1f("roughness", roughness);

        for (int i = 0; i < 6; ++i) {
            preShader->SetUniformMat4f("view", captureViews[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            skyboxCube->GetVertexArray()->Bind();
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    // 4. BRDF LUT
    glGenTextures(1, &brdfLUTTexture);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    std::shared_ptr<Shader> brdfShader = renderManager->GetShader("brdf");
    if (brdfShader) {
        brdfShader->Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderQuad();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    // 상태 복구
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
    
    glClearColor(oldClearColor.r, oldClearColor.g, oldClearColor.b, oldClearColor.a);

    int w = Engine::GetInstance().GetWindowWidth();
    int h = Engine::GetInstance().GetWindowHeight();
    glViewport(0, 0, w, h);
}

void Skybox::Render(Camera* camera)
{
    std::shared_ptr<Shader> skyShader = Engine::GetInstance().GetRenderManager()->GetShader("skybox_render");

    if (skyShader && skyboxCube && envCubemap != 0)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);

        skyShader->Bind();
        skyShader->SetUniformMat4f("view", camera->GetViewMatrix());
        skyShader->SetUniformMat4f("projection", camera->GetProjectionMatrix());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        skyShader->SetUniform1i("environmentMap", 0);

        skyboxCube->GetVertexArray()->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
        skyboxCube->GetVertexArray()->UnBind();

        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
    }
}

void Skybox::BindIBL()
{
    glActiveTexture(GL_TEXTURE0 + static_cast<int>(TextureSlot::IBL_IRRADIANCE));
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

    glActiveTexture(GL_TEXTURE0 + static_cast<int>(TextureSlot::IBL_PREFILTER));
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);

    glActiveTexture(GL_TEXTURE0 + static_cast<int>(TextureSlot::IBL_BRDF_LUT));
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
}

void Skybox::RenderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}