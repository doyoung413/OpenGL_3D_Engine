#pragma once
#include "Scene.hpp"
#include <memory>
#include "Skybox.hpp"

class PBRScene : public Scene
{
public:
    PBRScene();
    virtual ~PBRScene();

    void Init() override;
    void Update(float dt) override;
    void Restart() override;
    void End() override;
    void PostRender(Camera* camera) override;
    void RenderImGui() override;

private:
    std::unique_ptr<Skybox> skybox;
};