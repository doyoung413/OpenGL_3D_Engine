#include "AnimationDemoScene.hpp"
#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "RenderManager.hpp"
#include "SceneManager.hpp"
#include "InputManager.hpp"
#include "CameraManager.hpp"
#include "MeshRenderer.hpp"
#include "Animator.hpp"
#include "AnimationStateMachine.hpp"
#include "Light.hpp"

AnimationDemoScene::AnimationDemoScene() {}
AnimationDemoScene::~AnimationDemoScene() {}

void AnimationDemoScene::Init()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();
    CameraManager* cameraManager = Engine::GetInstance().GetCameraManager();

    // 셰이더 및 리소스 로드
    renderManager->LoadShader("basic", "asset/shaders/basic.vert", "asset/shaders/basic.frag");
    renderManager->LoadShader("pbr", "asset/shaders/pbr.vert", "asset/shaders/pbr.frag");
    renderManager->LoadTexture("wall", "asset/wall.jpg");

    // 스카이박스 설정
    skybox = std::make_unique<Skybox>();
    skybox->Init("asset/hdr/modern_evening_street_4k.hdr");

    // 바닥 (Plane) 생성
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(0), [&](Object* object) {
        object->SetName("Ground");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreatePlane();
        renderer->SetShader("basic");
        renderer->SetTexture("wall");
        // 바닥을 조금 더 넓게 설정
        object->transform.SetScale(20.0f, 1.0f, 20.0f);
        object->transform.SetPosition(0.0f, 0.0f, 0.0f);
    });

    // 조명 (Directional Light) - 캐릭터를 비추기 위함
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(1), [&](Object* object) {
        object->SetName("Sun Light");
        auto lightComp = object->AddComponent<Light>();
        lightComp->SetColor({ 1.0f, 0.95f, 0.8f }); // 약간 따뜻한 햇빛 색
        lightComp->SetType(LightType::Directional);
        lightComp->SetDirection({ -0.5f, -1.0f, -0.5f }); // 위에서 아래로 비스듬히
    });

    // 춤추는 캐릭터 (DancingObject)
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(2), [](Object* object) {
        object->SetName("Michael"); // 이름 변경
        object->transform.SetPosition(0.0f, 1.f, 0.0f); // 중앙 배치
        object->transform.SetRotationY(180.f); // 카메라를 바라보게 회전
        object->transform.SetScale(0.01f, 0.01f, 0.01f); // 모델 크기에 맞춰 조정

        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->LoadModel("asset/models/character.fbx", "mixamorig:Hips");
        renderer->SetShader("basic");

        auto animator = object->AddComponent<Animator>();
        animator->SetEnableRootMotion(true); // 루트 모션 활성화
        animator->SetBakeOptions({ false, false, false, false });

        // FSM 설정
        if (renderer->GetModel())
        {
            auto fsm = object->AddComponent<AnimationStateMachine>();
            fsm->AddState("Thriller_1", "asset/models/Thriller_1.fbx");
            fsm->AddState("Thriller_2", "asset/models/Thriller_2.fbx");
            fsm->AddState("Thriller_3", "asset/models/Thriller_3.fbx");
            fsm->AddState("Thriller_4", "asset/models/Thriller_4.fbx");

            // 첫 상태 시작 (루프 끔)
            fsm->ChangeState("Thriller_1", false);
        }
    });

    //  제자리 애니메이션 캐릭터 (Punching Guy)
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(3), [](Object* object) {
        object->SetName("PunchingGuy");
        object->transform.SetPosition(-2.0f, 0.0f, 0.0f); // X = -2.0 (왼쪽)
        object->transform.SetRotationY(180.f);
        object->transform.SetScale(0.01f, 0.01f, 0.01f);

        auto renderer = object->AddComponent<MeshRenderer>();
        // MeshesScene에서 사용하던 모델 파일 로드
        renderer->LoadModel("asset/models/Test.fbx", "mixamorig:Hips");
        renderer->SetShader("basic");

        object->AddComponent<Animator>(); // Animator 추가 (RootMotion 미사용)

        if (renderer->GetModel())
        {
            auto fsm = object->AddComponent<AnimationStateMachine>();

            fsm->AddState("Idle", "asset/models/Idle.fbx");
            fsm->AddState("Walk", "asset/models/Walking_1.fbx");
            fsm->AddState("Punch", "asset/models/Quad Punch.fbx");
            fsm->AddState("Dance", "asset/models/Swing Dancing.fbx");

            // 기본 상태로 Punch 설정 (Loop)
            fsm->ChangeState("Punch");
        }
    });

    // 카메라 설정
    cameraManager->ClearCameras();
    int index = cameraManager->CreateCamera();
    Camera* camera = cameraManager->GetCamera(index);

    // 캐릭터 전신이 잘 보이도록 카메라 위치 조정
    camera->SetCameraPosition(glm::vec3{ 0.0f, 1.5f, -10.0f });
    camera->SetTarget({ 0.0f, 1.0f, 0.0f }); // 캐릭터의 허리쯤을 바라봄

    cameraManager->SetMainCamera(index);

    objectManager->Init();
}

void AnimationDemoScene::Update(float dt)
{
    HandleCameraInput(dt);

    if (skybox) skybox->BindIBL();

    // 애니메이션 시퀀스 로직 (Thriller 1 -> 2 -> 3 -> 4 -> 1 반복)
    Object* character = Engine::GetInstance().GetObjectManager()->FindObjectByName("Michael");
    if (character)
    {
        auto fsm = character->GetComponent<AnimationStateMachine>();
        auto animator = character->GetComponent<Animator>();

        if (fsm && animator)
        {
            // 현재 애니메이션이 끝났다면(Stopped), 다음 동작으로 넘어감
            if (animator->GetPlaybackState() == PlaybackState::Stopped)
            {
                std::string current = fsm->GetCurrentStateName();

                // 블렌딩 시간(0.25f)을 주어 부드럽게 연결
                if (current == "Thriller_1")      fsm->ChangeState("Thriller_2", false, 1.0f, 0.25f);
                else if (current == "Thriller_2") fsm->ChangeState("Thriller_3", false, 1.0f, 0.25f);
                else if (current == "Thriller_3") fsm->ChangeState("Thriller_4", false, 1.0f, 0.25f);
                else if (current == "Thriller_4") fsm->ChangeState("Thriller_1", false, 1.0f, 0.25f);
            }
        }
    }
}

void AnimationDemoScene::HandleCameraInput(float dt)
{
    Camera* mainCamera = Engine::GetInstance().GetCameraManager()->GetMainCamera();
    InputManager* input = Engine::GetInstance().GetInputManager();
    if (!mainCamera || !input) return;

    const float cameraSpeed = 2.5f * dt;

    if (input->IsKeyPressed(KEYBOARDKEYS::W)) mainCamera->MoveCameraPos(CameraMoveDir::FORWARD, cameraSpeed);
    if (input->IsKeyPressed(KEYBOARDKEYS::S)) mainCamera->MoveCameraPos(CameraMoveDir::BACKWARD, cameraSpeed);
    if (input->IsKeyPressed(KEYBOARDKEYS::A)) mainCamera->MoveCameraPos(CameraMoveDir::LEFT, cameraSpeed);
    if (input->IsKeyPressed(KEYBOARDKEYS::D)) mainCamera->MoveCameraPos(CameraMoveDir::RIGHT, cameraSpeed);
    if (input->IsKeyPressed(KEYBOARDKEYS::SPACE)) mainCamera->MoveCameraPos(CameraMoveDir::UP, cameraSpeed);
    if (input->IsKeyPressed(KEYBOARDKEYS::LSHIFT)) mainCamera->MoveCameraPos(CameraMoveDir::DOWN, cameraSpeed);

    // 마우스 우클릭 시에만 회전하도록 설정 (편의성)
    if (input->IsMouseButtonPressed(MOUSEBUTTON::RIGHT))
    {
        input->SetRelativeMouseMode(true);
        glm::vec2 mouseDelta = input->GetRelativeMouseMotion();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            mainCamera->UpdateCameraDirection(mouseDelta);
        }
    }
    else
    {
        input->SetRelativeMouseMode(false);
    }
}

void AnimationDemoScene::PostRender(Camera* camera)
{
    if (skybox) skybox->Render(camera);
    Engine::GetInstance().GetObjectManager()->RenderGizmos(camera);
    Engine::GetInstance().GetObjectManager()->RenderBoneHierarchy(camera);
}

void AnimationDemoScene::RenderImGui()
{
    Engine::GetInstance().GetObjectManager()->ObjectControllerForImgui();
    Engine::GetInstance().GetCameraManager()->CameraControllerForImGui();
}

void AnimationDemoScene::Restart() {}

void AnimationDemoScene::End()
{
    Engine::GetInstance().GetObjectManager()->DestroyAllObjects();
    Engine::GetInstance().GetRenderManager()->ResetAllResources();
    Engine::GetInstance().GetCameraManager()->ClearCameras();
}