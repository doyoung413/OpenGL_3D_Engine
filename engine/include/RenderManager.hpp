#pragma once
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include "glm.hpp"

enum class TextureSlot
{
    // Material (0~4)
    ALBEDO = 0,
    NORMAL = 1,
    METALLIC = 2,
    ROUGHNESS = 3,
    AO = 4,

    // Shadow (5~9)
    SHADOW_MAP = 5,

    // Global IBL (10~12)
    IBL_IRRADIANCE = 10,
    IBL_PREFILTER = 11,
    IBL_BRDF_LUT = 12
};

class Engine;
class Shader;  
class Texture;
class MeshRenderer;
class Light;

class RenderManager
{
public:
    RenderManager() = default;
    ~RenderManager() = default;
    RenderManager(const RenderManager&) = delete;
    RenderManager& operator=(const RenderManager&) = delete;

    void Register(MeshRenderer* renderer);
    void Unregister(MeshRenderer* renderer);

    void RegisterLight(Light* light);
    void UnregisterLight(Light* light);

    void BeginFrame();
    void Render();
    void EndFrame();

    void SetClearColor(glm::vec4 color) { backGroundColor = color; }
    void ProcessQueues();

    std::shared_ptr<Shader> LoadShader(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    std::shared_ptr<Shader> GetShader(const std::string& name);

    std::shared_ptr<Texture> LoadTexture(const std::string& name, const std::string& filePath);
    std::shared_ptr<Texture> GetTexture(const std::string& name);

    void ResetShaders();
    void ResetTextures();
    void ResetAllResources();
private:
    friend class Engine;
    std::vector<MeshRenderer*> renderers;
    std::vector<MeshRenderer*> pendingAddition;
    std::vector<MeshRenderer*> pendingRemoval;
    std::vector<Light*> lights;
    glm::vec4 backGroundColor = { 0.1f, 0.1f, 0.15f, 1.0f };

    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
};
