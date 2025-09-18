#pragma once
#include "Scene.hpp"

class MeshesScene : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Restart() override;
    void End() override;

    void HandleInputTests();
};