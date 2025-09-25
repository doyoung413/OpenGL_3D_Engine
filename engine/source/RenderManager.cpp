#include "RenderManager.hpp"
#include "Engine.hpp"  
#include "MeshRenderer.hpp"
#include "CameraManager.hpp"
#include "Shader.hpp"  
#include "Texture.hpp" 
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
    if (!cameraManager) return;

    // CameraManager가 관리하는 모든 카메라 리스트를 가져옴
    auto& cameraList = cameraManager->GetCameraList();

    // 등록된 모든 카메라에 대해 업데이트
    for (const auto& cameraPtr : cameraList)
    {
        Camera* camera = cameraPtr.get();
        if (!camera) continue;

        glm::vec4 vp = camera->GetRelativeViewport();

        // OpenGL의 뷰포트를 설정
        glViewport(static_cast<int>(vp.x * static_cast<float>(windowWidth)), static_cast<int>(vp.y * static_cast<float>(windowHeight)),
            static_cast<int>(vp.z * static_cast<float>(windowWidth)), static_cast<int>(vp.w * static_cast<float>(windowHeight)) );

        // 깊이 버퍼를 초기화
        glClear(GL_DEPTH_BUFFER_BIT);

        // 모든 객체를 현재 카메라의 시점에서 렌더링
        for (MeshRenderer* renderer : renderers)
        {
            renderer->Render(camera);
        }
    }

    // 렌더링이 끝난 후, 다음 프레임이나 UI 렌더링을 위해 뷰포트를 전체 화면으로 되돌림
    glViewport(0, 0, windowWidth, windowHeight);
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