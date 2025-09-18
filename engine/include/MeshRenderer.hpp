#pragma once
#include "Component.hpp"
#include "Mesh.hpp"
#include <memory>

class Shader;
class Texture;
class Camera;

enum class RenderMode { Fill, Wireframe };

class MeshRenderer : public Component
{
public:
    MeshRenderer() : Component(ComponentTypes::MESHRENDERER) {}

    void Init() override;
    void Update(float dt) override;
    void End() override;

    void Render();
    void Render(Camera* camera);

    void CreatePlane();
    void CreateCube();
    void CreateSphere();
    void CreateDiamond();
    void CreateCylinder();
    void CreateCapsule();
    void CreateFromData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitivePattern pattern);

    void SetShader(const std::string& name);
    void SetTexture(const std::string& name);

    void SetRenderMode(RenderMode mode) { renderMode = mode; }

    void SetColor(const glm::vec4& color_) { color = color_; }
    const glm::vec4& GetColor() const { return color; }

private:
    std::unique_ptr<Mesh> mesh;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> texture;
    RenderMode renderMode = RenderMode::Fill;

    glm::vec4 color = glm::vec4(1.0f);
};
