#pragma once
#include <memory>
#include "Scene.hpp"
#include "glm.hpp"

class Object;
class Mesh;
class Shader;
struct aiNode;
class MeshesScene : public Scene
{
public:
    MeshesScene();         
    virtual ~MeshesScene();

    void Init() override;
    void Update(float dt) override;
    void Restart() override;
    void End() override;
    void PostRender(Camera* camera) override;
    void RenderImGui() override;

private:
    void HandleInputTests();
    void HandleCameraInput(float dt);

    std::unique_ptr<Mesh> boneMesh;
    std::shared_ptr<Shader> debugShader;

    // ������ ��� ������
    bool bDrawSkeleton = false;
    bool isWeightDebugMode = false;

    // ���� ��������� �׸��� ���� �Լ�
    void DrawBoneHierarchy(const aiNode* node, const glm::mat4& parentTransform, Camera* camera);
};