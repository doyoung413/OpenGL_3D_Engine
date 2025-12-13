#include "SceneManager.hpp"
#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "RenderManager.hpp"
#include "CameraManager.hpp"
#include "Scene.hpp"
#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

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

void SceneManager::ImGuiBeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

// ImGui의 렌더링 데이터를 최종적으로 화면에 그리는 로직을 모아둡니다.
void SceneManager::ImGuiEndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SceneManager::RenderMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Select Demo"))
        {
            if (ImGui::MenuItem("Meshes Demo"))
            {
                ChangeScene(SceneTag::MESHES);
            }

            if (ImGui::MenuItem("Animation Demo"))
            {
                ChangeScene(SceneTag::ANIMATION_DEMO);
            }

            if (ImGui::MenuItem("PBR"))
            {
                ChangeScene(SceneTag::PBR);
            }

            if (ImGui::MenuItem("Game Demo"))
            {
                ChangeScene(SceneTag::GAME);
            }

            if (ImGui::MenuItem("MoCap"))
            {
                ChangeScene(SceneTag::MOCAP);
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
                ChangeState(SceneState::SHUTDOWN);
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
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
            ImGuiBeginFrame();
            RenderMainMenuBar();

            ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
            RenderManager* renderManager = Engine::GetInstance().GetRenderManager();
            currentScene->Update(dt);

            Engine::GetInstance().GetCameraManager()->Update(dt);

            objectManager->Update(dt);

            renderManager->BeginFrame();
            renderManager->Render();

            currentScene->RenderImGui();
            ImGuiEndFrame();
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