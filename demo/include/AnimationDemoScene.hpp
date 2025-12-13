#pragma once
#include "Scene.hpp"
#include <memory>
#include "Skybox.hpp"

class AnimationDemoScene : public Scene
{
public:
    AnimationDemoScene();
    virtual ~AnimationDemoScene();

    void Init() override;
    void Update(float dt) override;
    void Restart() override;
    void End() override;
    void PostRender(Camera* camera) override;
    void RenderImGui() override;

private:
    void HandleCameraInput(float dt);

    std::unique_ptr<Skybox> skybox;
};