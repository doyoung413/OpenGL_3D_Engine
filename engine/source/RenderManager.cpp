#include "RenderManager.hpp"
#include "Engine.hpp"  
#include "MeshRenderer.hpp"
#include "CameraManager.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp" 
#include "Shader.hpp"  
#include "Texture.hpp" 
#include "Light.hpp" 
#include <algorithm>

void RenderManager::Register(MeshRenderer* renderer)
{
    pendingAddition.push_back(renderer);
}

void RenderManager::Unregister(MeshRenderer* renderer)
{
    pendingRemoval.push_back(renderer);
}

void RenderManager::BeginFrame()
{
    glClearColor(backGroundColor.x, backGroundColor.y, backGroundColor.z, backGroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderManager::Render()
{
    int windowWidth = Engine::GetInstance().GetWindowWidth();
    int windowHeight = Engine::GetInstance().GetWindowHeight();
    CameraManager* cameraManager = Engine::GetInstance().GetCameraManager();
    Scene* currentScene = Engine::GetInstance().GetSceneManager()->GetCurrentScene(); // 현재 씬을 가져옴

    if (!cameraManager) return;

    // 깊이 버퍼 쓰기/테스트 허용
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 면 제거(Culling) 끄기
    glDisable(GL_CULL_FACE);

    // 블렌딩 켜기
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // CameraManager가 관리하는 모든 카메라 리스트를 가져wkf옴
    auto& cameraList = cameraManager->GetCameraList();

    // 등록된 모든 카메라에 대해 업데이트
    for (const auto& cameraPtr : cameraList)
    {
        Camera* camera = cameraPtr.get();
        if (!camera) continue;

        glm::vec4 vp = camera->GetRelativeViewport();

        // OpenGL의 뷰포트를 설정
        glViewport(static_cast<int>(vp.x * static_cast<float>(windowWidth)), static_cast<int>(vp.y * static_cast<float>(windowHeight)),
            static_cast<int>(vp.z * static_cast<float>(windowWidth)), static_cast<int>(vp.w * static_cast<float>(windowHeight)));

        // 깊이 버퍼를 초기화
        glClear(GL_DEPTH_BUFFER_BIT);

        // 모든 객체를 현재 카메라의 시점에서 렌더링
        if (lights.empty())
        {
            // 빛 없이 모든 객체를 그림
            for (MeshRenderer* renderer : renderers)
            {
                renderer->Render(camera, nullptr);
            }
        }
        else
        {
            int i = 0;
            for (Light* light : lights)
            {
                if (!light) continue;

                // 첫 번째 조명이 아닐 경우, 가산 블렌딩을 활성화
                if (i > 0)
                {
                    glEnable(GL_BLEND);
                    // 덧셈 방식으로 색상을 혼합 (기존 색상 + 새로 그릴 색상)
                    glBlendFunc(GL_ONE, GL_ONE);
                    // 깊이 값이 같아도 그리도록 설정 (하이라이트가 겹치도록)
                    glDepthFunc(GL_LEQUAL);
                }

                // 현재 빛의 영향을 받아 모든 객체를 렌더링
                for (MeshRenderer* renderer : renderers)
                {
                    renderer->Render(camera, light);
                }

                // 가산 블렌딩을 사용했다면, 다음 카메라 뷰를 위해 상태를 원상 복구
                if (i > 0)
                {
                    glDisable(GL_BLEND);
                    glDepthFunc(GL_LESS); // 기본 깊이 테스트 함수로 복구
                }
                i++;
            }
        }
        currentScene->PostRender(camera);

        // 렌더링이 끝난 후, 다음 프레임이나 UI 렌더링을 위해 뷰포트를 전체 화면으로 되돌림
        glViewport(0, 0, windowWidth, windowHeight);
    }
}

void RenderManager::EndFrame()
{
    SDL_GL_SwapWindow(Engine::GetInstance().GetSDLWindow());
}

void RenderManager::ProcessQueues()
{
    if (!pendingRemoval.empty()) {
        renderers.erase(
            std::remove_if(renderers.begin(), renderers.end(),
                [&](MeshRenderer* renderer) {
            return std::find(pendingRemoval.begin(), pendingRemoval.end(), renderer) != pendingRemoval.end();
        }),
            renderers.end()
        );
        pendingRemoval.clear();
    }
    if (!pendingAddition.empty()) {
        renderers.insert(renderers.end(), pendingAddition.begin(), pendingAddition.end());
        pendingAddition.clear();
    }
}

std::shared_ptr<Shader> RenderManager::LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath)
{
    if (shaders.count(name)) {
        return shaders[name];
    }
    auto shader = std::make_shared<Shader>(vertexPath, fragmentPath);
    shaders[name] = shader;
    return shader;
}

std::shared_ptr<Shader> RenderManager::GetShader(const std::string& name)
{
    if (shaders.count(name)) {
        return shaders[name];
    }
    return nullptr;
}

std::shared_ptr<Texture> RenderManager::LoadTexture(const std::string& name, const std::string& filePath)
{
    if (textures.count(name)) {
        return textures[name];
    }
    auto texture = std::make_shared<Texture>(filePath);
    textures[name] = texture;
    return texture;
}

std::shared_ptr<Texture> RenderManager::GetTexture(const std::string& name)
{
    if (textures.count(name)) {
        return textures[name];
    }
    return nullptr;
}

void RenderManager::ResetShaders()
{
    shaders.clear();
}

void RenderManager::ResetTextures()
{
    textures.clear();
}

void RenderManager::ResetAllResources()
{
    ResetShaders();
    ResetTextures();
}

void RenderManager::RegisterLight(Light* light)
{
    lights.push_back(light);
}

void RenderManager::UnregisterLight(Light* light)
{
    auto it = std::find(lights.begin(), lights.end(), light);
    if (it != lights.end())
    {
        lights.erase(it);
    }
}