#pragma once
#include "Component.hpp"
#include "Mesh.hpp"
#include <memory>

class Shader;
class Texture;
class Camera;
class Model;
class Light;

enum class RenderMode { Fill, Wireframe };

class MeshRenderer : public Component
{
public:
    MeshRenderer() : Component(ComponentTypes::MESHRENDERER) {}

    void Init() override;
    void Update(float dt) override;
    void End() override;

    void Render(Camera* camera, Light* light);

    void CreatePlane();
    void CreateCube();
    void CreateSphere();
    void CreateDiamond();
    void CreateCylinder();
    void CreateCapsule();
    void CreateFromData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitivePattern pattern);

    void SetShader(const std::string& name);
    void SetShader(std::shared_ptr<Shader> shader_);
    std::shared_ptr<Shader> GetShader() { return shader; }
    void SetTexture(const std::string& name);

    // 모델 로딩 함수
    void LoadModel(const std::string& path, const std::string& customRootBoneName = "");
    Model* GetModel() const { return model.get(); }

    void SetRenderMode(RenderMode mode) { renderMode = mode; }

    void SetColor(const glm::vec4& color_) { color = color_; }
    const glm::vec4& GetColor() const { return color; }

    void SetMetallic(float metallic_) { metallic = metallic_; }
    float GetMetallic() { return metallic; }
    void SetRoughness(float roughness_) { roughness = roughness_; }
    float GetRoughness() { return roughness; }
    void SetExposure(float exp) { exposure = exp; }
    float GetExposure() const { return exposure; }
private:
    std::shared_ptr<Model> model; // 모델 파일 로딩용
    std::unique_ptr<Mesh> mesh;   // CreateCube 등 절차적 생성용
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> texture;
    RenderMode renderMode = RenderMode::Fill;

    glm::vec4 color = glm::vec4(1.0f);
    float metallic = 0.5f;
    float roughness = 0.5f; 
    float exposure = 0.5f;
};
