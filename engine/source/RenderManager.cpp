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
    Scene* currentScene = Engine::GetInstance().GetSceneManager()->GetCurrentScene(); // ���� ���� ������

    if (!cameraManager) return;

    // CameraManager�� �����ϴ� ��� ī�޶� ����Ʈ�� ������
    auto& cameraList = cameraManager->GetCameraList();

    // ��ϵ� ��� ī�޶� ���� ������Ʈ
    for (const auto& cameraPtr : cameraList)
    {
        Camera* camera = cameraPtr.get();
        if (!camera) continue;

        glm::vec4 vp = camera->GetRelativeViewport();

        // OpenGL�� ����Ʈ�� ����
        glViewport(static_cast<int>(vp.x * static_cast<float>(windowWidth)), static_cast<int>(vp.y * static_cast<float>(windowHeight)),
            static_cast<int>(vp.z * static_cast<float>(windowWidth)), static_cast<int>(vp.w * static_cast<float>(windowHeight)));

        // ���� ���۸� �ʱ�ȭ
        glClear(GL_DEPTH_BUFFER_BIT);

        // ��� ��ü�� ���� ī�޶��� �������� ������
        if (lights.empty())
        {
            // �� ���� ��� ��ü�� �׸�
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

                // ù ��° ������ �ƴ� ���, ���� ������ Ȱ��ȭ
                if (i > 0)
                {
                    glEnable(GL_BLEND);
                    // ���� ������� ������ ȥ�� (���� ���� + ���� �׸� ����)
                    glBlendFunc(GL_ONE, GL_ONE);
                    // ���� ���� ���Ƶ� �׸����� ���� (���̶���Ʈ�� ��ġ����)
                    glDepthFunc(GL_LEQUAL);
                }

                // ���� ���� ������ �޾� ��� ��ü�� ������
                for (MeshRenderer* renderer : renderers)
                {
                    renderer->Render(camera, light);
                }

                // ���� ������ ����ߴٸ�, ���� ī�޶� �並 ���� ���¸� ���� ����
                if (i > 0)
                {
                    glDisable(GL_BLEND);
                    glDepthFunc(GL_LESS); // �⺻ ���� �׽�Ʈ �Լ��� ����
                }
                i++;
            }
        }
        currentScene->PostRender(camera);

        // �������� ���� ��, ���� �������̳� UI �������� ���� ����Ʈ�� ��ü ȭ������ �ǵ���
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