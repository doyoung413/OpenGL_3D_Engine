#include "GameScene.hpp"
#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "RenderManager.hpp"
#include "InputManager.hpp"
#include "SceneManager.hpp"
#include "CameraManager.hpp"
#include "MeshRenderer.hpp"
#include "Animator.hpp"
#include "AnimationStateMachine.hpp"
#include "Light.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/quaternion.hpp> 
#include <algorithm>

GameScene::GameScene() {}
GameScene::~GameScene() {}

void GameScene::Init()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();

    renderManager->LoadShader("pbr", "asset/shaders/pbr.vert", "asset/shaders/pbr.frag");
    renderManager->LoadShader("basic", "asset/shaders/basic.vert", "asset/shaders/basic.frag");

    skybox = std::make_unique<Skybox>();
    skybox->Init("asset/hdr/modern_evening_street_4k.hdr");

    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(0), [&](Object* obj) {
        obj->SetName("Ground");
        auto renderer = obj->AddComponent<MeshRenderer>();
        renderer->CreatePlane();
        renderer->SetShader("pbr");
        renderer->SetColor({ 0.2f, 0.2f, 0.2f, 1.0f });
        renderer->SetMetallic(0.2f);
        renderer->SetRoughness(0.8f);
        obj->transform.SetScale(20.0f, 1.0f, 20.0f);
        obj->transform.SetPosition(0.0f, 0.0f, 0.0f);
    });

    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(1), [&](Object* obj) {
        obj->SetName("Player");
        obj->transform.SetPosition(0.0f, 0.0f, 0.0f);
        obj->transform.SetScale(0.01f, 0.01f, 0.01f);

        auto renderer = obj->AddComponent<MeshRenderer>();
        renderer->LoadModel("asset/models/character.fbx", "mixamorig:Hips");
        renderer->SetShader("pbr");

        obj->AddComponent<Animator>();
        auto fsm = obj->AddComponent<AnimationStateMachine>();

        fsm->AddState("Idle", "asset/models/Idle.fbx");
        fsm->AddState("Run", "asset/models/Walking_1.fbx");
        fsm->ChangeState("Idle");

        playerObject = obj;
    });

    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(2), [&](Object* obj) {
        obj->SetName("Sun Light");
        auto light = obj->AddComponent<Light>();
        light->SetType(LightType::Directional);
        light->SetColor({ 1.0f, 0.95f, 0.8f });
        light->SetDirection({ -0.5f, -1.0f, -0.5f });
    });

    CameraManager* camManager = Engine::GetInstance().GetCameraManager();
    int camIdx = camManager->CreateCamera();
    Camera* mainCam = camManager->GetCamera(camIdx);

    mainCam->SetIsThirdPersonView(true);
    mainCam->SetCameraDistance(3.5f);   
    mainCam->SetCameraOffset({ 0.0f, 3.f, 0.0f });
    mainCam->SetCameraPosition(glm::vec3{ 0.0f, 2.0f, 5.0f });
    mainCam->SetPitch(-20.0f);

    camManager->SetMainCamera(camIdx);

    int coinCount = 8;
    float radius = 5.0f;

    for (int i = 0; i < coinCount; ++i)
    {
        float angle = (float)i * (360.0f / coinCount);
        float rad = glm::radians(angle);

        float x = cos(rad) * radius;
        float z = sin(rad) * radius;

        CreateCoin(glm::vec3(x, 1.0f, z));
    }

    totalCoins = coinCount;
    score = 0;

    objectManager->Init();
}

void GameScene::Update(float dt)
{
    if (skybox) skybox->BindIBL();
    HandlePlayerInput(dt);

    if (playerObject)
    {
        Camera* cam = Engine::GetInstance().GetCameraManager()->GetMainCamera();
        cam->SetTarget(playerObject->transform.GetPosition());
    }

    // 코인 애니메이션 (빙글빙글 돌기)
    for (Object* coin : coins)
    {
        float currentRotY = coin->transform.GetRotation().y;
        coin->transform.SetRotationY(currentRotY + 180.0f * dt);
    }
    CheckCoinCollisions();
}

void GameScene::HandlePlayerInput(float dt)
{
    if (!playerObject) return;

    InputManager* input = Engine::GetInstance().GetInputManager();
    auto fsm = playerObject->GetComponent<AnimationStateMachine>();

    glm::vec3 moveDir(0.0f);

    glm::vec3 inputDir(0.0f);
    if (input->IsKeyPressed(KEYBOARDKEYS::W)) inputDir.z += 1.0f;
    if (input->IsKeyPressed(KEYBOARDKEYS::S)) inputDir.z -= 1.0f;
    if (input->IsKeyPressed(KEYBOARDKEYS::A)) inputDir.x += 1.0f;
    if (input->IsKeyPressed(KEYBOARDKEYS::D)) inputDir.x -= 1.0f;

    if (input->IsMouseButtonPressed(MOUSEBUTTON::RIGHT))
    {
        input->SetRelativeMouseMode(true);
    }
    if (input->IsMouseButtonReleasedOnce(MOUSEBUTTON::RIGHT))
    {
        input->SetRelativeMouseMode(false);
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::F11))
    {
        Engine::GetInstance().ToggleFullscreen();
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::ESCAPE))
    {
        Engine::GetInstance().GetSceneManager()->ChangeState(SceneState::SHUTDOWN);
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::NUMBER_3))
    {
        Engine::GetInstance().ToggleMSAA();
    }

    // 입력이 있으면 정규화
    if (glm::length(inputDir) > 0.0f)
    {
        inputDir = glm::normalize(inputDir);
    }

    bool isMoving = (glm::length(inputDir) > 0.0f);
    float targetSpeed = isMoving ? maxMoveSpeed : 0.0f;

    // 멈출 때는 더 빠르게 감속 (20.0f), 출발할 때는 부드럽게 (5.0f)
    float accel = isMoving ? acceleration : 20.0f;

    // 선형 보간으로 속도 조절 (dt 적용)
    currentSpeed += (targetSpeed - currentSpeed) * accel * dt;

    // 아주 작은 속도는 0으로 고정 (미세 떨림 방지)
    if (std::abs(currentSpeed) < 0.01f) currentSpeed = 0.0f;

    if (isMoving)
    {
        UpdateSmoothRotation(inputDir, dt);
    }

    // 캐릭터가 현재 바라보는 방향으로 전진합니다.
    // 회전이 먼저 적용되었으므로, 자연스럽게 곡선을 그리며 이동합니다.

    // 현재 캐릭터의 Y축 회전값(Yaw)

    glm::vec3 moveDirection;
    if (isMoving)
    {
        moveDirection = inputDir; // 키 누르는 중: 입력 방향으로 이동
    }
    else
    {
        // 키 뗐을 때: 바라보는 방향으로 미끄러지듯 정지
        float yawRad = glm::radians(playerObject->transform.GetRotation().y);
        moveDirection = glm::vec3(sin(yawRad), 0.0f, cos(yawRad));
    }

    glm::vec3 currentPos = playerObject->transform.GetPosition();
    currentPos += moveDirection * currentSpeed * dt;
    playerObject->transform.SetPosition(currentPos);

    if (fsm)
    {
        // 속도가 조금이라도 붙으면 Run
        if (currentSpeed > 0.1f)
        {
            if (fsm->GetCurrentStateName() != "Run")
                fsm->ChangeState("Run", true, 1.0f, 0.1f); // 0.1초 블렌딩
        }
        // 멈추면 Idle
        else
        {
            if (fsm->GetCurrentStateName() != "Idle")
                fsm->ChangeState("Idle", true, 1.0f, 0.2f);
        }
    }


    if (input->GetRelativeMouseMode())
    {
        Camera* mainCamera = Engine::GetInstance().GetCameraManager()->GetMainCamera();
        glm::vec2 mouseDelta = input->GetRelativeMouseMotion();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            mainCamera->UpdateCameraDirection(mouseDelta);
        }
    }
}

void GameScene::UpdateSmoothRotation(const glm::vec3& inputDir, float dt)
{
    // 1. 목표 각도 계산 (atan2: x, z)
    // 2. [중요] + glm::pi<float>()를 추가하여 모델을 180도 돌립니다.
    float targetAngleRad = atan2(inputDir.x, inputDir.z);

    // 3. 쿼터니언 생성
    glm::quat targetRotation = glm::angleAxis(targetAngleRad, glm::vec3(0.0f, 1.0f, 0.0f));

    // 4. 현재 회전값
    glm::vec3 currentEuler = playerObject->transform.GetRotation();
    glm::quat currentRotation = glm::quat(glm::radians(currentEuler));

    // 5. Slerp 보간 (회전 속도 10.0f)
    float rotSpeed = 10.0f;
    glm::quat nextRotation = glm::slerp(currentRotation, targetRotation, rotSpeed * dt);

    // 6. 적용
    playerObject->transform.SetRotation(glm::degrees(glm::eulerAngles(nextRotation)));
}

void GameScene::CreateCoin(glm::vec3 position)
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();

    objectManager->AddObject<Object>();
    auto& objList = objectManager->GetObjectList();
    Object* coinObj = objList.back().get();

    objectManager->QueueObjectFunction(coinObj, [position](Object* obj) {
        obj->SetName("Coin");
        auto renderer = obj->AddComponent<MeshRenderer>();
        renderer->CreateCylinder();
        renderer->SetShader("pbr");

        renderer->SetColor({ 1.0f, 0.84f, 0.0f, 1.0f });
        renderer->SetMetallic(1.0f);
        renderer->SetRoughness(0.2f);

        obj->transform.SetPosition(position);
        obj->transform.SetScale(0.5f, 0.05f, 0.5f);
        obj->transform.SetRotationX(90.0f);
    });
    coins.push_back(coinObj);
}

void GameScene::CheckCoinCollisions()
{
    if (!playerObject) return;

    glm::vec3 playerPos = playerObject->transform.GetPosition();
    float collisionDistance = 1.5f;

    auto it = coins.begin();
    while (it != coins.end())
    {
        Object* coin = *it;
        glm::vec3 coinPos = coin->transform.GetPosition();
        float distXZ = glm::distance(glm::vec2(playerPos.x, playerPos.z), glm::vec2(coinPos.x, coinPos.z));

        if (distXZ < collisionDistance)
        {
            std::cout << "Collected!" << std::endl;
            score++;
            Engine::GetInstance().GetObjectManager()->DestroyObject(coin);
            it = coins.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void GameScene::PostRender(Camera* camera)
{
    if (skybox) skybox->Render(camera);
    Engine::GetInstance().GetObjectManager()->RenderGizmos(camera);
}

void GameScene::Restart() {}
void GameScene::End()
{
    Engine::GetInstance().GetObjectManager()->DestroyAllObjects();
    Engine::GetInstance().GetRenderManager()->ResetAllResources();
    Engine::GetInstance().GetCameraManager()->ClearCameras();
}
void GameScene::RenderImGui()
{
    Engine::GetInstance().GetObjectManager()->ObjectControllerForImgui();
    ImGui::SetNextWindowPos(ImVec2(20, 50), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);

    ImGui::SetNextWindowBgAlpha(0.6f);
    if (ImGui::Begin("Game Status", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("SCORE");
        ImGui::Separator();

        std::string scoreText = std::to_string(score) + " / " + std::to_string(totalCoins);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
        ImGui::SetWindowFontScale(2.0f);
        ImGui::Text("%s", scoreText.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        if (score == totalCoins && totalCoins > 0)
        {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "CLEAR!");
            if (ImGui::Button("Restart Game"))
            {
                Engine::GetInstance().GetSceneManager()->ChangeScene(SceneTag::GAME);
            }
        }
    }
    ImGui::End();
}