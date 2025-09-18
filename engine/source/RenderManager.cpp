#include "RenderManager.hpp"
#include "Engine.hpp"  
#include "MeshRenderer.hpp"
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
    for (MeshRenderer* renderer : renderers)
    {
        renderer->Render();
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
    return nullptr; // 없을 경우 nullptr 반환
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