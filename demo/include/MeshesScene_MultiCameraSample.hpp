#pragma once
#include "Scene.hpp"

class Object;
class MeshesScene_MultiCameraSample : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Restart() override;
    void End() override;

private:
    void HandleInputTests();
    void HandleCameraInput(float dt);
    void HandlePlayerControl(float dt);

    Object* controlledPlayer = nullptr;
};