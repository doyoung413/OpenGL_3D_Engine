#pragma once
#include <memory>
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
};