#include "MeshesScene.hpp"

#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "MeshRenderer.hpp"
#include "SceneManager.hpp"
#include "RenderManager.hpp"
#include "InputManager.hpp"
#include "CameraManager.hpp"
#include "Light.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

#include "Animator.hpp"
#include "Animation.hpp"
#include "AnimationStateMachine.hpp"
#include "imgui.h"

MeshesScene::MeshesScene() = default;
MeshesScene::~MeshesScene() = default;

void MeshesScene::Init()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();

    renderManager->LoadShader("basic", "asset/shaders/basic.vert", "asset/shaders/basic.frag");
    renderManager->LoadShader("weight_debug", "asset/shaders/weight_debug.vert", "asset/shaders/weight_debug.frag");
    renderManager->LoadShader("pbr", "asset/shaders/pbr.vert", "asset/shaders/pbr.frag");

    // IBL 사전 연산용 셰이더 로드
    renderManager->LoadShader("skybox", "asset/shaders/skybox.vert", "asset/shaders/equirectangular.frag"); // Pass 1용
    renderManager->LoadShader("irradiance", "asset/shaders/skybox.vert", "asset/shaders/irradiance.frag"); // Pass 2용
    renderManager->LoadShader("prefilter", "asset/shaders/skybox.vert", "asset/shaders/prefilter.frag"); // Pass 3용

    renderManager->LoadTexture("wall", "asset/wall.jpg");
    renderManager->LoadTexture("container", "asset/container.jpg");
    renderManager->LoadTexture("backpack", "asset/models/backpack/diffuse.jpg");

    // HDR 텍스처 로드 (경로는 실제 파일에 맞게 수정)
    std::string hdrPath = "asset/hdr/modern_evening_street_4k.hdr";
    renderManager->LoadTexture("hdr_env", hdrPath);

    // [추가] IBL 사전 연산 실행
    PrecomputeIBL(hdrPath);

    // 평면
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(0), [&](Object* object) {
        object->SetName("Plane");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreatePlane();
        renderer->SetShader("basic");
        renderer->SetTexture("wall");
        object->transform.SetPosition(0.0f, -0.75f, 0.0f);
        object->transform.SetScale(10.0f, 1.0f, 10.0f);
    });

    // 원기둥
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(1), [&](Object* object) {
        object->SetName("Cylinder");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateCylinder();
        renderer->SetShader("basic");
        renderer->SetRenderMode(RenderMode::Wireframe);
        renderer->SetColor({ 0.0f, 1.f, 0.0f, 1.0f });
        object->transform.SetPosition(-3.0f, 0.0f, 0.0f);
    });

    // 구 (Pbr 테스트)
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(2), [&](Object* object) {
        object->SetName("Sphere");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateSphere();
        renderer->SetShader("pbr"); // "basic" -> "pbr"
        renderer->SetColor({ 0.8f, 0.8f, 1.0f, 1.0f });
        object->transform.SetPosition(-1.5f, 0.0f, 0.0f);
    });

    // 큐브
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(3), [&](Object* object) {
        object->SetName("Cube");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateCube();
        renderer->SetShader("basic");
        renderer->SetTexture("container");
        object->transform.SetPosition(0.0f, 0.0f, 0.0f);
    });

    // 다이아몬드
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(4), [&](Object* object) {
        object->SetName("Diamond");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateDiamond();
        renderer->SetShader("basic");
        renderer->SetColor({ 1.0f, 0.8f, 0.2f, 1.0f });
        object->transform.SetPosition(1.5f, 0.0f, 0.0f);
    });

    // CreateFromData 테스트용 피라미드 생성
    std::vector<Vertex> pyramidVertices = {
        // 각 정점에 법선(normal) 데이터를 추가합니다.
        // 위치,                     법선(계산된 값),         색상,                   텍스처 좌표
        Vertex{ {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {0.2f, 0.3f, 0.8f}, {0.0f, 0.0f} }, // 0
        Vertex{ { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, {0.2f, 0.3f, 0.8f}, {1.0f, 0.0f} }, // 1
        Vertex{ { 0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, {0.2f, 0.3f, 0.8f}, {1.0f, 1.0f} }, // 2
        Vertex{ {-0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f}, {0.2f, 0.3f, 0.8f}, {0.0f, 1.0f} }, // 3
        Vertex{ { 0.0f,  0.5f,  0.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f, 0.2f}, {0.5f, 0.5f} }  // 4
    };

    std::vector<unsigned int> pyramidIndices = {
        // 밑면
        0, 1, 2,   2, 3, 0,
        // 옆면
        0, 4, 1,   // 앞면
        1, 4, 2,   // 오른쪽 면
        2, 4, 3,   // 뒷면
        3, 4, 0    // 왼쪽 면
    };

    objectManager->AddObject<Object>();
        // [&] -> [=] 또는 [변수, 변수1....] => 람다가 변수들의 복사본 가짐
    objectManager->QueueObjectFunction(objectManager->FindObject(5),
        [pyramidVertices, pyramidIndices](Object* object) {
        object->SetName("Pyramid");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateFromData(pyramidVertices, pyramidIndices, PrimitivePattern::Triangles);
        renderer->SetShader("basic");
        renderer->SetTexture("container");
        object->transform.SetPosition(3.0f, -0.25f, 0.0f);
    });

    //모델 불러오기
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(6), [&](Object* object) {
        object->SetName("Backpack");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->LoadModel("asset/models/backpack/backpack.obj");
        renderer->SetShader("basic");
        renderer->SetTexture("backpack");
        object->transform.SetPosition(0.0f, 1.2f, 0.0f);
        object->transform.SetScale(0.3f, 0.3f, 0.3f);
    });

    // 붉은색 조명
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(7), [&](Object* object) {
        object->SetName("Red Light");
        object->transform.SetPosition({ -2.0f, 1.0f, 2.0f });

        auto lightComp = object->AddComponent<Light>();
        lightComp->SetColor({ 1.0f, 0.0f, 0.0f });
        lightComp->SetType(LightType::Point);

        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateSphere();
        renderer->SetShader("basic");
        renderer->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
        object->transform.SetScale(0.2f, 0.2f, 0.2f);
    });

    // 푸른색 조명
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(8), [&](Object* object) {
        object->SetName("Blue Light");
        object->transform.SetPosition({ 2.0f, 1.0f, 2.0f });

        auto lightComp = object->AddComponent<Light>();
        lightComp->SetColor({ 0.0f, 0.0f, 1.0f });
        lightComp->SetType(LightType::Point);

        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateSphere();
        renderer->SetShader("basic");
        renderer->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f });
        object->transform.SetScale(0.2f, 0.2f, 0.2f);
    });

    // 초록색 조명
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(9), [&](Object* object) {
        object->SetName("Green Light");
        object->transform.SetPosition({ 0.0f, 1.0f, -2.0f });

        auto lightComp = object->AddComponent<Light>();
        lightComp->SetColor({ 0.0f, 1.0f, 0.0f });
        //lightComp->SetOffsetForPointL({ 0.0f, 0.0f, 2.0f });
        lightComp->SetType(LightType::Point);

        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateSphere();
        renderer->SetShader("basic");
        renderer->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
        object->transform.SetScale(0.2f, 0.2f, 0.2f);
    });

    // 직접 조명
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(10), [&](Object* object) {
        object->SetName("Direction Light");
        object->transform.SetPosition({ 0.0f, 1.0f, -2.0f });

        auto lightComp = object->AddComponent<Light>();
        lightComp->SetColor({ 1.0f, 1.0f, 1.0f });
        lightComp->SetType(LightType::Directional);
        lightComp->SetDirection({ -0.2f, -1.0f, -0.3f });
    });

    //애니메이션 오브젝트
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(11), [this](Object* object) {
        object->SetName("AnimationObject");
        object->transform.SetPosition(0.0f, -0.5f, -1.25f);
        object->transform.SetRotationY(180.f);
        object->transform.SetScale(0.01f, 0.01f, 0.01f);

        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->LoadModel("asset/models/Test.fbx", "mixamorig:Hips");
        renderer->SetShader("basic");
        object->AddComponent<Animator>();
        Model* model = renderer->GetModel();
        if (model)
        {
            auto fsm = object->AddComponent<AnimationStateMachine>();

            fsm->AddState("Walk", "asset/models/Walking_1.fbx");
            fsm->AddState("Punch", "asset/models/Quad Punch.fbx");
            fsm->AddState("Dance", "asset/models/Swing Dancing.fbx");
            fsm->ChangeState("Punch");
        }
    });

    //춤 애니메이션 오브젝트
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(12), [this](Object* object) {
        object->SetName("DancingObject");
        object->transform.SetPosition(2.0f, 0.5f, -1.25f);
        object->transform.SetRotationY(180.f);
        object->transform.SetScale(0.01f, 0.01f, 0.01f);

        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->LoadModel("asset/models/character.fbx", "mixamorig:Hips");
        renderer->SetShader("basic");
        auto animator = object->AddComponent<Animator>();
        animator->SetEnableRootMotion(true);
        Model* model = renderer->GetModel();
        if (model)
        {
            auto fsm = object->AddComponent<AnimationStateMachine>();

            fsm->AddState("Thriller_1", "asset/models/Thriller_1.fbx");
            fsm->AddState("Thriller_2", "asset/models/Thriller_2.fbx");
            fsm->AddState("Thriller_3", "asset/models/Thriller_3.fbx");
            fsm->AddState("Thriller_4", "asset/models/Thriller_4.fbx");
            fsm->ChangeState("Thriller_1", false);
        }
    });


    CameraManager* cameraManager = Engine::GetInstance().GetCameraManager();
    int index = cameraManager->CreateCamera();
    Camera* camera = cameraManager->GetCamera(index);
    camera->SetCameraPosition(glm::vec3{ 0.f,0.f,-3.f });
    cameraManager->SetMainCamera(index);

    objectManager->Init();
}

void MeshesScene::Update(float dt)
{
    HandleInputTests();
    HandleCameraInput(dt);

    Object* character = Engine::GetInstance().GetObjectManager()->FindObjectByName("DancingObject");
    if (character)
    {
        auto fsm = character->GetComponent<AnimationStateMachine>();
        auto animator = character->GetComponent<Animator>();


        if (fsm->GetCurrentStateName() == "Thriller_1" && animator->GetPlaybackState() == PlaybackState::Stopped)
        {
            fsm->ChangeState("Thriller_2", false, 1.0f, 0.25f);
        }
        else if (fsm->GetCurrentStateName() == "Thriller_2" && animator->GetPlaybackState() == PlaybackState::Stopped)
        {
            fsm->ChangeState("Thriller_3", false, 1.0f, 0.25f);
        }
        else if (fsm->GetCurrentStateName() == "Thriller_3" && animator->GetPlaybackState() == PlaybackState::Stopped)
        {
            fsm->ChangeState("Thriller_4", false, 1.0f, 0.25f);
        }
        else if (fsm->GetCurrentStateName() == "Thriller_4" && animator->GetPlaybackState() == PlaybackState::Stopped)
        {
            fsm->ChangeState("Thriller_1", false, 1.0f, 0.25f);
        }
    }
}
void MeshesScene::Restart()
{
}

void MeshesScene::End()
{
    // IBL 유틸리티 메쉬 메모리 해제
    skyboxCube.reset();

    // IBL 텍스처 메모리 해제
    glDeleteTextures(1, &envCubemap);
    glDeleteTextures(1, &irradianceMap);
    glDeleteTextures(1, &prefilterMap);
    envCubemap = 0;
    irradianceMap = 0;
    prefilterMap = 0;

    Engine::GetInstance().GetObjectManager()->DestroyAllObjects();
    Engine::GetInstance().GetRenderManager()->ResetAllResources();
}

void MeshesScene::HandleInputTests()

{
    InputManager* input = Engine::GetInstance().GetInputManager();

    if (input->IsKeyPressOnce(KEYBOARDKEYS::W))
    {
        std::cout << "[Key Press Once] W" << std::endl;
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::S))
    {
        std::cout << "[Key Held Down] S" << std::endl;
    }
    if (input->IsKeyReleasedOnce(KEYBOARDKEYS::A))
    {
        std::cout << "[Key Released] A" << std::endl;
    }
    if (input->IsMouseButtonPressOnce(MOUSEBUTTON::LEFT))
    {
        glm::vec2 pos = input->GetMousePosition();
        std::cout << "[Mouse Click] Left Button at (" << pos.x << ", " << pos.y << ")" << std::endl;
    }
    glm::vec2 wheelMotion = input->GetMouseWheelMotion();
    if (wheelMotion.y != 0)
    {
        std::cout << "[Mouse Wheel] Scrolled, Y-motion: " << wheelMotion.y << std::endl;
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::R))
    {
        bool currentMode = input->GetRelativeMouseMode();
        input->SetRelativeMouseMode(!currentMode);
        std::cout << "[System] Relative Mouse Mode Toggled to " << (!currentMode ? "ON" : "OFF") << std::endl;
    }
    if (input->GetRelativeMouseMode())
    {
        glm::vec2 motion = input->GetRelativeMouseMotion();
        if (motion.x != 0 || motion.y != 0)
        {
            std::cout << "[Mouse Motion] Relative delta (" << motion.x << ", " << motion.y << ")" << std::endl;
        }
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
}

void MeshesScene::HandleCameraInput(float dt)
{
    Camera* mainCamera = Engine::GetInstance().GetCameraManager()->GetMainCamera();
    if (!mainCamera)
    {
        return;
    }

    InputManager* input = Engine::GetInstance().GetInputManager();
    const float cameraSpeed = 2.5f * dt;

    if (input->IsKeyPressed(KEYBOARDKEYS::W)) 
    {
        mainCamera->MoveCameraPos(CameraMoveDir::FORWARD, cameraSpeed);
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::S)) 
    {
        mainCamera->MoveCameraPos(CameraMoveDir::BACKWARD, cameraSpeed);
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::A)) 
    {
        mainCamera->MoveCameraPos(CameraMoveDir::LEFT, cameraSpeed);
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::D)) 
    {
        mainCamera->MoveCameraPos(CameraMoveDir::RIGHT, cameraSpeed);
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::SPACE)) 
    {
        mainCamera->MoveCameraPos(CameraMoveDir::UP, cameraSpeed);
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::LSHIFT)) 
    {
        mainCamera->MoveCameraPos(CameraMoveDir::DOWN, cameraSpeed);
    }

    // 상대 마우스 모드가 켜져있을 때만 시점 변경
    if (input->GetRelativeMouseMode())
    {
        glm::vec2 mouseDelta = input->GetRelativeMouseMotion();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            mainCamera->UpdateCameraDirection(mouseDelta);
        }
    }
}

void MeshesScene::PostRender(Camera* camera)
{
    Engine::GetInstance().GetObjectManager()->RenderGizmos(camera);
    Engine::GetInstance().GetObjectManager()->RenderBoneHierarchy(camera);

}

void MeshesScene::RenderImGui()
{
    Engine::GetInstance().GetObjectManager()->ObjectControllerForImgui();
}

void MeshesScene::PrecomputeIBL(const std::string& hdrTexturePath)
{
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();
    Texture* hdrTexture = renderManager->GetTexture("hdr_env").get();
    if (!hdrTexture) {
        std::cerr << "Failed to load HDR texture for IBL." << std::endl;
        return;
    }

    // 유틸리티 생성: FBO, RBO, 큐브 메쉬
    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512); // 큐브맵 최대 해상도
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

    // 큐브 메쉬 생성 (CreateFromData 사용)
    std::vector<Vertex> skyboxVertices = {
        {{-1.0f,  1.0f, -1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
        {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},
        {{-1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{-1.0f,  1.0f, -1.0f}},
        {{-1.0f,  1.0f, -1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}},
        {{ 1.0f, -1.0f, -1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
        {{-1.0f, -1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}, {{-1.0f, -1.0f,  1.0f}},
        {{-1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f, -1.0f}}, {{ 1.0f,  1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f,  1.0f}}, {{-1.0f,  1.0f, -1.0f}},
        {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f, -1.0f}},
        {{ 1.0f, -1.0f, -1.0f}}, {{-1.0f, -1.0f,  1.0f}}, {{ 1.0f, -1.0f,  1.0f}}
    };
    std::vector<unsigned int> skyboxIndices(36);
    for (int i = 0; i < 36; ++i) skyboxIndices[i] = i;
    skyboxCube = std::make_shared<Mesh>(skyboxVertices, skyboxIndices);
    skyboxCube->UploadToGPU(); // VAO 생성

    // 환경 큐브맵 (envCubemap) 생성
    unsigned int envMapSize = 512;
    glGenTextures(1, &envCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, envMapSize, envMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 밉맵 사용
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 캡처용 View / Projection 행렬 설정
    glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    glm::mat4 captureViews[] = {
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
       glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
    };

    // (Pass 1) Equirectangular -> Cubemap
    std::shared_ptr<Shader> equirectangularShader = renderManager->GetShader("skybox");
    equirectangularShader->Bind();
    equirectangularShader->SetUniformMat4f("projection", captureProjection);
    hdrTexture->Bind(0);
    equirectangularShader->SetUniform1i("equirectangularMap", 0);

    glViewport(0, 0, envMapSize, envMapSize);
    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    for (int i = 0; i < 6; ++i) {
        equirectangularShader->SetUniformMat4f("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skyboxCube->GetVertexArray()->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP); // 밉맵 생성

    // (Pass 2) Irradiance Map 생성
    unsigned int irradianceMapSize = 32;
    glGenTextures(1, &irradianceMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, irradianceMapSize, irradianceMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, irradianceMapSize, irradianceMapSize);
    glViewport(0, 0, irradianceMapSize, irradianceMapSize);

    std::shared_ptr<Shader> irradianceShader = renderManager->GetShader("irradiance");
    irradianceShader->Bind();
    irradianceShader->SetUniformMat4f("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); // Pass 1의 결과물
    irradianceShader->SetUniform1i("environmentMap", 0);

    for (int i = 0; i < 6; ++i) {
        irradianceShader->SetUniformMat4f("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        skyboxCube->GetVertexArray()->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // (Pass 3) Pre-filtered Specular Map 생성
    unsigned int prefilterMapSize = 128;
    glGenTextures(1, &prefilterMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
    for (int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, prefilterMapSize, prefilterMapSize, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 밉맵 레벨간 선형보간
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP); // 밉맵 레벨 생성

    std::shared_ptr<Shader> prefilterShader = renderManager->GetShader("prefilter");
    prefilterShader->Bind();
    prefilterShader->SetUniformMat4f("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap); // Pass 1의 결과물
    prefilterShader->SetUniform1i("environmentMap", 0);

    unsigned int maxMipLevels = 5;
    for (int mip = 0; mip < maxMipLevels; ++mip) {
        // 밉맵 레벨에 따라 뷰포트 크기 조절
        unsigned int mipWidth = static_cast<unsigned int>(prefilterMapSize * std::pow(0.5, mip));
        unsigned int mipHeight = static_cast<unsigned int>(prefilterMapSize * std::pow(0.5, mip));
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader->SetUniform1f("roughness", roughness);

        for (int i = 0; i < 6; ++i) {
            prefilterShader->SetUniformMat4f("view", captureViews[i]);
            // 밉맵 레벨 'mip'에 렌더링
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            skyboxCube->GetVertexArray()->Bind();
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
    }

    // 정리
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    // 뷰포트 원상복구
    int w = Engine::GetInstance().GetWindowWidth();
    int h = Engine::GetInstance().GetWindowHeight();
    glViewport(0, 0, w, h);

    std::cout << "[IBL] Pre-computation finished. envCubemap, irradianceMap, prefilterMap generated." << std::endl;
}