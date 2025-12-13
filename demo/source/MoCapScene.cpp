#include "MoCapScene.hpp"

#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "RenderManager.hpp"
#include "CameraManager.hpp"
#include "MeshRenderer.hpp"
#include "InputManager.hpp"
#include "Light.hpp"
#include "MotionCaptureSystem.hpp" 

#include "imgui.h"
#include <iostream>
#include <glew.h> 

#define GLM_ENABLE_EXPERIMENTAL
#include <gtc/quaternion.hpp>
#include <gtx/vector_angle.hpp>

MoCapScene::MoCapScene() {}
MoCapScene::~MoCapScene()
{
    // 텍스처 리소스 해제
    if (debugTextureID != 0)
    {
        glDeleteTextures(1, &debugTextureID);
    }
}

void MoCapScene::Init()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();
    CameraManager* camManager = Engine::GetInstance().GetCameraManager();

    // 셰이더 로드
    renderManager->LoadShader("basic", "asset/shaders/basic.vert", "asset/shaders/basic.frag");

    // 모션 캡쳐 시스템 생성
    mocapSystem = std::make_unique<MotionCaptureSystem>();

    // 경로 확인
    if (!mocapSystem->Init("asset/motionCapture/pose_landmark_full.onnx")) {
        std::cerr << "Failed to init MoCap System!" << std::endl;
    }

    // 관절(구체) 생성
    CreateJoints();
    // 뼈(원기둥) 생성
    CreateBones();

    // 조명 추가
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(0), [&](Object* obj) {
        obj->SetName("Sun Light");
        auto light = obj->AddComponent<Light>();
        light->SetType(LightType::Directional);
        light->SetColor({ 1.0f, 1.0f, 1.0f });
        light->SetDirection({ -0.5f, -1.0f, -0.5f });
    });

    // 카메라 설정
    camManager->ClearCameras();
    int camIdx = camManager->CreateCamera();
    Camera* mainCam = camManager->GetCamera(camIdx);

    // 정면에서 바라보도록 설정
    mainCam->SetCameraPosition({ 0.5f, 0.4f, 9.8f });
    mainCam->SetYaw(265.f);

    camManager->SetMainCamera(camIdx);

    // 오브젝트 매니저 초기화
    objectManager->Init();
}

void MoCapScene::CreateJoints()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    jointSpheres.clear();
    jointSpheres.resize(33);

    // BlazePose는 33개의 랜드마크를 반환함
    for (int i = 0; i < 33; ++i)
    {
        objectManager->AddObject<Object>();
        auto& objList = objectManager->GetObjectList();
        Object* jointObj = objList.back().get();

        objectManager->QueueObjectFunction(jointObj, [i, this](Object* obj) {
            std::string name = "Joint_" + std::to_string(i);
            obj->SetName(name);

            auto renderer = obj->AddComponent<MeshRenderer>();
            renderer->CreateSphere(); // 구체 생성
            renderer->SetShader("basic");
            renderer->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f }); // 빨간색

            // 초기 크기는 작게 설정
            obj->transform.SetScale(0.1f, 0.1f, 0.1f);

            // 리스트에 저장해두고 Update에서 위치 제어
            jointSpheres[i] = obj;
        });
    }
}

void MoCapScene::CreateBones()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    boneCylinders.clear();

    for (const auto& pair : boneConnections)
    {
        objectManager->AddObject<Object>();
        Object* boneObj = objectManager->GetObjectList().back().get();

        objectManager->QueueObjectFunction(boneObj, [this](Object* obj) {
            obj->SetName("Bone");
            auto renderer = obj->AddComponent<MeshRenderer>();
            renderer->CreateCylinder();

            renderer->SetShader("basic");
            renderer->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
            obj->transform.SetScale(0.05f, 1.0f, 0.05f);

            boneCylinders.push_back(obj);
        });
    }
}

void MoCapScene::Update(float dt)
{
    // 모션 캡쳐 업데이트
    if (mocapSystem)
    {
        mocapSystem->Update();
        SyncJoints(); // 구체 위치 동기화
        SyncBones(); // 뼈대 업데이트
    }

    // 카메라 조작 (WASD + 마우스 우클릭)
    Camera* mainCam = Engine::GetInstance().GetCameraManager()->GetMainCamera();
    InputManager* input = Engine::GetInstance().GetInputManager();

    if (mainCam && input)
    {
        float speed = 2.5f * dt;
        if (input->IsKeyPressed(KEYBOARDKEYS::W)) mainCam->MoveCameraPos(CameraMoveDir::FORWARD, speed);
        if (input->IsKeyPressed(KEYBOARDKEYS::S)) mainCam->MoveCameraPos(CameraMoveDir::BACKWARD, speed);
        if (input->IsKeyPressed(KEYBOARDKEYS::A)) mainCam->MoveCameraPos(CameraMoveDir::LEFT, speed);
        if (input->IsKeyPressed(KEYBOARDKEYS::D)) mainCam->MoveCameraPos(CameraMoveDir::RIGHT, speed);

        if (input->IsMouseButtonPressOnce(MOUSEBUTTON::RIGHT)) input->SetRelativeMouseMode(true);
        if (input->IsMouseButtonReleasedOnce(MOUSEBUTTON::RIGHT)) input->SetRelativeMouseMode(false);

        if (input->GetRelativeMouseMode()) {
            mainCam->UpdateCameraDirection(input->GetRelativeMouseMotion());
        }
    }
}

void MoCapScene::SyncJoints()
{
    const auto& landmarks = mocapSystem->GetLandmarks();
    if (landmarks.empty()) return;

    // 가시성(Visibility) 필터링 로직
    float leftShoulderVis = landmarks[11].visibility;
    float rightShoulderVis = landmarks[12].visibility;
    float leftHipVis = landmarks[23].visibility;
    float rightHipVis = landmarks[24].visibility;

    const float THRESHOLD = 0.6f;
    bool isUpperBodyVisible = (leftShoulderVis > THRESHOLD || rightShoulderVis > THRESHOLD);
    bool isFullBodyVisible = (leftHipVis > THRESHOLD || rightHipVis > THRESHOLD);

    for (int i = 0; i < 33; ++i)
    {
        if (i >= jointSpheres.size() || jointSpheres[i] == nullptr) continue;

        const auto& data = landmarks[i];
        bool shouldShow = true;

        // 머리만 나오는 경우 (어깨 안 보임 -> 몸통 숨김)
        if (!isUpperBodyVisible && i >= 11) shouldShow = false;

        // 상반신만 나오는 경우 (골반 안 보임 -> 다리 숨김)
        else if (!isFullBodyVisible && (i >= 25 && i <= 32)) shouldShow = false;

        // 개별 신뢰도 체크
        if (data.visibility < THRESHOLD) shouldShow = false;

        if (!shouldShow)
        {
            jointSpheres[i]->transform.SetScale(0.0f, 0.0f, 0.0f);
        }
        else
        {
            jointSpheres[i]->transform.SetScale(0.1f, 0.1f, 0.1f);

            glm::vec3 finalPos = data.position;
            // AI 좌표 보정 (필요에 따라 스케일이나 오프셋 조정)
            // 예: 1.5배 증폭
            finalPos *= 1.5f;
            finalPos.y += 1.0f; // 높이 보정

            jointSpheres[i]->transform.SetPosition(finalPos);
        }
    }
}

void MoCapScene::SyncBones()
{
    if (boneCylinders.empty()) return;

    // 뼈 연결 정보와 오브젝트 리스트의 인덱스는 순서가 같음
    for (size_t i = 0; i < boneConnections.size(); ++i)
    {
        if (i >= boneCylinders.size()) break;

        Object* bone = boneCylinders[i];
        int idx1 = boneConnections[i].first;
        int idx2 = boneConnections[i].second;

        Object* sphere1 = jointSpheres[idx1];
        Object* sphere2 = jointSpheres[idx2];

        // 연결된 두 구체 중 하나라도 안 보이면(Scale이 0이면) 뼈도 숨김
        if (sphere1->transform.GetScale().x < 0.001f || sphere2->transform.GetScale().x < 0.001f)
        {
            bone->transform.SetScale(0.0f, 0.0f, 0.0f);
            continue;
        }

        glm::vec3 pos1 = sphere1->transform.GetPosition();
        glm::vec3 pos2 = sphere2->transform.GetPosition();

        // 위치: 두 점의 중점
        glm::vec3 centerPos = (pos1 + pos2) * 0.5f;
        bone->transform.SetPosition(centerPos);

        // 회전: 기본 Y축(Up) 벡터를 (pos2 - pos1) 방향으로 회전
        glm::vec3 direction = pos2 - pos1;
        float length = glm::length(direction);

        // 길이가 너무 짧으면 숨김
        if (length < 0.01f)
        {
            bone->transform.SetScale(0.0f, 0.0f, 0.0f);
            continue;
        }

        // 기본 원기둥이 Y축(0,1,0) 방향으로 서 있다고 가정
        glm::quat rotation = GetRotationBetweenVectors(glm::vec3(0.0f, 1.0f, 0.0f), direction);
        bone->transform.SetRotation(glm::degrees(glm::eulerAngles(rotation)));

        // (원기둥 기본 높이가 1.0m라고 가정. 만약 2.0m라면 length / 2.0f 해야 함)
        bone->transform.SetScale(0.05f, length / 2.0f, 0.05f);
    }
}

void MoCapScene::ResetSkeletonState()
{
    // 스케일을 0으로 만들어 화면에서 숨깁니다.
    for (Object* sphere : jointSpheres)
    {
        if (sphere)
        {
            sphere->transform.SetScale(0.0f, 0.0f, 0.0f);
        }
    }

    for (Object* bone : boneCylinders)
    {
        if (bone)
        {
            bone->transform.SetScale(0.0f, 0.0f, 0.0f);
        }
    }
}

glm::quat MoCapScene::GetRotationBetweenVectors(glm::vec3 sourceVector, glm::vec3 targetVector)
{
    sourceVector = glm::normalize(sourceVector);
    targetVector = glm::normalize(targetVector);

    float dot = glm::dot(sourceVector, targetVector);
    if (dot > 0.999f) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

    float angle = acos(glm::clamp(dot, -1.0f, 1.0f));
    glm::vec3 axis = glm::cross(sourceVector, targetVector);

    if (glm::length(axis) < 0.001f) axis = glm::vec3(0.0f, 1.0f, 0.0f);
    else axis = glm::normalize(axis);

    return glm::angleAxis(angle, axis);
}

void MoCapScene::Restart() 
{
    End();
}

void MoCapScene::End()
{
    Engine::GetInstance().GetObjectManager()->DestroyAllObjects();
    Engine::GetInstance().GetRenderManager()->ResetAllResources();
    Engine::GetInstance().GetCameraManager()->ClearCameras();

    jointSpheres.clear();
    boneCylinders.clear();

    if (mocapSystem)
    {
        mocapSystem.reset();
    }
}

void MoCapScene::PostRender(Camera* camera)
{
    Engine::GetInstance().GetObjectManager()->RenderGizmos(camera);
}

void MoCapScene::RenderImGui()
{
    Engine::GetInstance().GetObjectManager()->ObjectControllerForImgui();
    Engine::GetInstance().GetCameraManager()->CameraControllerForImGui();
    ImGui::Begin("Motion Capture Controller");

    if (ImGui::Button("Open Webcam")) {
        mocapSystem->OpenSource(InputMode::Webcam);
        ResetSkeletonState();
    }
    ImGui::SameLine();
    if (ImGui::Button("Open Video File")) {
        mocapSystem->OpenSource(InputMode::VideoFile);
        ResetSkeletonState();
    }

    ImGui::Separator();

    GLuint texID = mocapSystem->GetTextureID();
    if (texID != 0)
    {
        ImGui::Text("Camera Feed:");
        ImGui::Image((ImTextureID)(intptr_t)texID, ImVec2(800, 600));
    }
    else
    {
        ImGui::Text("No Video Source.");
    }

    ImGui::Separator();

    const auto& landmarks = mocapSystem->GetLandmarks();
    if (!landmarks.empty())
    {
        glm::vec3 nose = landmarks[0].position;
        ImGui::Text("Nose(World): (%.2f, %.2f, %.2f)", nose.x, nose.y, nose.z);
    }

    ImGui::End();
}