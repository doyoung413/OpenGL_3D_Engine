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
    void SetTexture(const std::string& name);

    // �� �ε� �Լ�
    void LoadModel(const std::string& path);
    Model* GetModel() const { return model.get(); }

    void SetRenderMode(RenderMode mode) { renderMode = mode; }

    void SetColor(const glm::vec4& color_) { color = color_; }
    const glm::vec4& GetColor() const { return color; }

private:
    std::shared_ptr<Model> model; // �� ���� �ε���
    std::unique_ptr<Mesh> mesh;   // CreateCube �� ������ ������
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Texture> texture;
    RenderMode renderMode = RenderMode::Fill;

    glm::vec4 color = glm::vec4(1.0f);
};
