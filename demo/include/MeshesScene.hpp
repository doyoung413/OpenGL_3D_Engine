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

    // 나머지 멤버 변수들
    bool bDrawSkeleton = false;
    bool isWeightDebugMode = false;

    // 뼈를 재귀적으로 그리는 헬퍼 함수
    void DrawBoneHierarchy(const aiNode* node, const glm::mat4& parentTransform, Camera* camera);
};