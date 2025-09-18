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
    // 씬 태그 순서대로 저장
    std::vector<Scene*> sceneList;

    Scene* currentScene = nullptr;
    SceneTag nextSceneTag = SceneTag::NONE;
    SceneState currentState = SceneState::START;
};