#include "SceneManager.hpp"
#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "RenderManager.hpp"
#include "CameraManager.hpp"
#include "Scene.hpp"
#include <iostream>

SceneManager::SceneManager() {}

SceneManager::~SceneManager()
{
    for (Scene* scene : sceneList)
    {
        delete scene;
    }
    sceneList.clear();
}

void SceneManager::Init(SceneTag startScene)
{
    nextSceneTag = startScene;
}

void SceneManager::AddScene(SceneTag tag, Scene* scene)
{
    if (static_cast<size_t>(tag) >= sceneList.size())
    {
        sceneList.resize(static_cast<size_t>(tag) + 1, nullptr);
    }
    sceneList[static_cast<int>(tag)] = scene;
}

void SceneManager::ChangeScene(SceneTag tag)
{
    nextSceneTag = tag;
    currentState = SceneState::CHANGE;
}

void SceneManager::ChangeState(SceneState state)
{
    currentState = state;
}

void SceneManager::Update(float dt)
{
    switch (currentState)
    {
    case SceneState::START:
        if (nextSceneTag == SceneTag::NONE) 
            currentState = SceneState::SHUTDOWN;
        else
        {

            std::cout << "Engine Start" << std::endl;
            currentState = SceneState::CHANGE;
        }

        break;

    case SceneState::LOAD:
        std::cout << "Scene Loaded" << std::endl;
        Engine::GetInstance().GetObjectManager()->DestroyAllObjects();
        Engine::GetInstance().GetRenderManager()->ResetAllResources();
        currentScene->Init();
        Engine::GetInstance().GetObjectManager()->ProcessQueues();
        Engine::GetInstance().GetRenderManager()->ProcessQueues();
        currentState = SceneState::UPDATE;
        std::cout << "Scene Update" << std::endl;
        break;

    case SceneState::UPDATE:
        if (nextSceneTag != SceneTag::NONE)
        {
            currentState = SceneState::CHANGE;
        }
        else
        {
            ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
            RenderManager* renderManager = Engine::GetInstance().GetRenderManager();
            currentScene->Update(dt);

            Engine::GetInstance().GetCameraManager()->Update(dt);

            objectManager->Update(dt);

            renderManager->BeginFrame();
            renderManager->Render();
            renderManager->EndFrame();

            objectManager->ProcessQueues();
            renderManager->ProcessQueues();
        }
        break;

    case SceneState::CHANGE:
        if (currentScene != nullptr) currentScene->End();
        currentScene = sceneList[static_cast<int>(nextSceneTag)];
        nextSceneTag = SceneTag::NONE;
        currentState = SceneState::LOAD;
        std::cout << "Scene Change" << std::endl;
        break;
    case SceneState::UNLOAD:
        std::cout << "Scene End" << std::endl;
        currentScene->End();

        Engine::GetInstance().GetObjectManager()->DestroyAllObjects();
        Engine::GetInstance().GetRenderManager()->ResetAllResources();
        Engine::GetInstance().GetCameraManager()->ClearCameras();
        currentState = SceneState::SHUTDOWN;
        break;

    case SceneState::SHUTDOWN:
        Engine::GetInstance().Quit();
        std::cout << "ShutDown" << std::endl;
        break;
    }
}