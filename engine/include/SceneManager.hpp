#pragma once
#include "SceneTag.hpp"
#include <vector>
#include <memory>

class Scene;
enum class SceneState
{
    START,
    LOAD,
    UPDATE,
    UNLOAD,
    CHANGE,
    SHUTDOWN
};

class SceneManager
{
public:
    SceneManager();
    ~SceneManager();

    void Init(SceneTag startScene);
    void Update(float dt);

    void AddScene(SceneTag tag, Scene* scene);
    void ChangeScene(SceneTag tag);

private:
    // �� �±� ������� ����
    std::vector<Scene*> sceneList;

    Scene* currentScene = nullptr;
    SceneTag nextSceneTag = SceneTag::NONE;
    SceneState currentState = SceneState::START;
};