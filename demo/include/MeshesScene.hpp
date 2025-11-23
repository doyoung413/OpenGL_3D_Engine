#pragma once
#include <memory>
#include <string>
#include "Scene.hpp"

class Object;
class Mesh;
class Shader;
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

    // IBL 사전 연산을 위한 헬퍼 함수
    void PrecomputeIBL(const std::string& hdrTexturePath);

    // IBL 텍스처 핸들 및 유틸리티
    unsigned int envCubemap = 0;
    unsigned int irradianceMap = 0;
    unsigned int prefilterMap = 0;
    std::shared_ptr<Mesh> skyboxCube;
};