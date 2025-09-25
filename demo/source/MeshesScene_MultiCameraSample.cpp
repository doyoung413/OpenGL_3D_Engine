#include "MeshesScene.hpp"
#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "MeshRenderer.hpp"
#include "RenderManager.hpp"
#include "InputManager.hpp"
#include "CameraManager.hpp"

void MeshesScene::Init()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();
    renderManager->LoadShader("basic", "asset/shaders/basic.vert", "asset/shaders/basic.frag");
    renderManager->LoadTexture("wall", "asset/wall.jpg");
    renderManager->LoadTexture("container", "asset/container.jpg");

    // ���
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(0), [&](Object* object) {
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreatePlane();
        renderer->SetShader("basic");
        renderer->SetTexture("wall");
        object->transform.SetPosition(0.0f, -0.75f, 0.0f);
        object->transform.SetScale(10.0f, 1.0f, 10.0f);
    });

    // �����
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(1), [&](Object* object) {
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateCylinder();
        renderer->SetShader("basic");
        renderer->SetRenderMode(RenderMode::Wireframe);
        renderer->SetColor({ 0.0f, 1.f, 0.0f, 1.0f });
        object->transform.SetPosition(-3.0f, 0.0f, 0.0f);
    });

    // ��
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(2), [&](Object* object) {
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateSphere();
        renderer->SetShader("basic");
        renderer->SetTexture("wall");
        renderer->SetColor({ 0.8f, 0.8f, 1.0f, 1.0f });
        object->transform.SetPosition(-1.5f, 0.0f, 0.0f);
    });

    // ť��
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(3), [&](Object* object) {
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateCube();
        renderer->SetShader("basic");
        renderer->SetTexture("container");
        object->transform.SetPosition(0.0f, 0.0f, 0.0f);
    });

    // ���̾Ƹ��
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(4), [&](Object* object) {
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateDiamond();
        renderer->SetShader("basic");
        renderer->SetColor({ 1.0f, 0.8f, 0.2f, 1.0f });
        object->transform.SetPosition(1.5f, 0.0f, 0.0f);
    });

    // CreateFromData �׽�Ʈ�� �Ƕ�̵� ����
    std::vector<Vertex> pyramidVertices = {
        // �ظ�
        { {-0.5f, -0.5f, -0.5f}, {0.2f, 0.3f, 0.8f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.2f, 0.3f, 0.8f}, {1.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.2f, 0.3f, 0.8f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.2f, 0.3f, 0.8f}, {0.0f, 1.0f} },
        // �����
        { { 0.0f,  0.5f,  0.0f}, {1.0f, 1.0f, 0.2f}, {0.5f, 0.5f} }
    };

    std::vector<unsigned int> pyramidIndices = {
        // �ظ�
        0, 1, 2,   2, 3, 0,
        // ����
        0, 4, 1,   // �ո�
        1, 4, 2,   // ������ ��
        2, 4, 3,   // �޸�
        3, 4, 0    // ���� ��
    };

    objectManager->AddObject<Object>();
        // [&] -> [=] �Ǵ� [����, ����1....] => ���ٰ� �������� ���纻 ����
    objectManager->QueueObjectFunction(objectManager->FindObject(5),
        [pyramidVertices, pyramidIndices](Object* object) {
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateFromData(pyramidVertices, pyramidIndices, PrimitivePattern::Triangles);
        renderer->SetShader("basic");
        renderer->SetTexture("container");
        object->transform.SetPosition(3.0f, -0.25f, 0.0f);
    });

    // ī�޶�� 1
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(6), [&](Object* object) {
        object->SetName("Cameraman 1");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateCapsule();
        renderer->SetShader("basic");
        renderer->SetColor({ 0.9f, 0.2f, 0.2f, 1.0f }); // ������
        object->transform.SetPosition(0.0f, 0.0f, 2.f);
        object->transform.SetScale(20.0f, 1.f, 0.5f);
    });

    // ī�޶�� 2
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(7), [&](Object* object) {
        object->SetName("Cameraman 2");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateSphere();
        renderer->SetShader("basic");
        renderer->SetColor({ 0.2f, 0.2f, 0.9f, 1.0f }); // Ǫ����
        object->transform.SetPosition(0.0f, 0.0f, -2.f);
        object->transform.SetScale(0.5f, 0.5f, 0.5f);
    });

    CameraManager* cameraManager = Engine::GetInstance().GetCameraManager();
    int windowWidth = Engine::GetInstance().GetWindowWidth();
    int windowHeight = Engine::GetInstance().GetWindowHeight();

    int mainCamIndex = cameraManager->CreateCamera();
    cameraManager->SetMainCamera(mainCamIndex);
    Camera* cam1 = cameraManager->GetCamera(mainCamIndex);
    cam1->SetViewport(0, 0, windowWidth / 2, windowHeight); // ȭ�� ���� ����
    cam1->SetCameraPosition({ 0.0f, 0.0f, 2.f });

    // �� ��° ī�޶� ���� �� ����
    int cam2Index = cameraManager->CreateCamera();
    Camera* cam2 = cameraManager->GetCamera(cam2Index);
    cam2->SetViewport(windowWidth / 2, 0, windowWidth / 2, windowHeight); // ȭ�� ������ ����
    cam2->SetCameraPosition({ 0.0f, 0.0f, -2.f });
}

void MeshesScene::Update(float dt)
{
    HandleInputTests();
    HandleCameraInput(dt);
    HandlePlayerControl(dt);
}

void MeshesScene::Restart()
{
}

void MeshesScene::End()
{
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

    // F11 Ű�� ���� ��üȭ�� ��� ���
    if (input->IsKeyPressOnce(KEYBOARDKEYS::F11))
    {
        Engine::GetInstance().ToggleFullscreen();
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::NUMBER_1))
    {
        Engine::GetInstance().GetCameraManager()->SetMainCamera(0);
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::NUMBER_2))
    {
        Engine::GetInstance().GetCameraManager()->SetMainCamera(1);
    }
}

void MeshesScene::HandleCameraInput(float dt)
{
    //Camera* mainCamera = Engine::GetInstance().GetCameraManager()->GetMainCamera();
    //if (!mainCamera)
    //{
    //    return;
    //}

    //InputManager* input = Engine::GetInstance().GetInputManager();
    //const float cameraSpeed = 0.5f * dt; // ������Ÿ��(dt)�� ���� �ӵ��� �����ϰ� ����

    //if (input->IsKeyPressed(KEYBOARDKEYS::W)) 
    //{
    //    mainCamera->MoveCameraPos(CameraMoveDir::FORWARD, cameraSpeed);
    //}
    //if (input->IsKeyPressed(KEYBOARDKEYS::S)) 
    //{
    //    mainCamera->MoveCameraPos(CameraMoveDir::BACKWARD, cameraSpeed);
    //}
    //if (input->IsKeyPressed(KEYBOARDKEYS::A)) 
    //{
    //    mainCamera->MoveCameraPos(CameraMoveDir::LEFT, cameraSpeed);
    //}
    //if (input->IsKeyPressed(KEYBOARDKEYS::D)) 
    //{
    //    mainCamera->MoveCameraPos(CameraMoveDir::RIGHT, cameraSpeed);
    //}
    //if (input->IsKeyPressed(KEYBOARDKEYS::SPACE)) 
    //{
    //    mainCamera->MoveCameraPos(CameraMoveDir::UP, cameraSpeed);
    //}
    //if (input->IsKeyPressed(KEYBOARDKEYS::LSHIFT)) 
    //{
    //    mainCamera->MoveCameraPos(CameraMoveDir::DOWN, cameraSpeed);
    //}

    //// ��� ���콺 ��尡 �������� ���� ���� ����
    //if (input->GetRelativeMouseMode())
    //{
    //    glm::vec2 mouseDelta = input->GetRelativeMouseMotion();
    //    if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
    //    {
    //        mainCamera->UpdateCameraDirection(mouseDelta);
    //    }
    //}

    Camera* mainCamera = Engine::GetInstance().GetCameraManager()->GetMainCamera();
    if (!mainCamera)
    {
        return;
    }

    InputManager* input = Engine::GetInstance().GetInputManager();
    const float cameraSpeed = 0.5f * dt; // �ӵ� ���� �ణ ����

    // --- ���Ⱑ �ٽ� ���� �κ� ---
    if (controlledPlayer != nullptr) // ������ �÷��̾ �ִٸ�
    {
     // ī�޶��� ����/������ ������ �������� �÷��̾ �̵���ŵ�ϴ�.
    const glm::vec3 forward = mainCamera->GetFrontVector();
    const glm::vec3 right = mainCamera->GetRightVector();
    glm::vec3 pos = controlledPlayer->transform.GetPosition();

    if (input->IsKeyPressed(KEYBOARDKEYS::W)) 
    {
        pos += glm::vec3(forward.x, 0.0f, forward.z) * cameraSpeed;
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::S)) 
    {
        pos -= glm::vec3(forward.x, 0.0f, forward.z) * cameraSpeed;
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::A)) 
    {
        pos -= glm::vec3(right.x, 0.0f, right.z) * cameraSpeed;
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::D)) 
    {
        pos += glm::vec3(right.x, 0.0f, right.z) * cameraSpeed;
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::SPACE)) 
    {
        pos.y += cameraSpeed;
    }
    if (input->IsKeyPressed(KEYBOARDKEYS::LSHIFT)) 
    {
        pos.y -= cameraSpeed;
    }

    controlledPlayer->transform.SetPosition(pos);
}
    else // ������ �÷��̾ ���ٸ� (���� ���� ���)
    {
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
    }

    // ���콺�� ���� �����ϴ� ������ �״�� ����
    if (input->GetRelativeMouseMode())
    {
        glm::vec2 mouseDelta = input->GetRelativeMouseMotion();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            mainCamera->UpdateCameraDirection(mouseDelta);
        }
    }
}


void MeshesScene::HandlePlayerControl(float /*dt*/)
{
    InputManager* input = Engine::GetInstance().GetInputManager();
    ObjectManager* objManager = Engine::GetInstance().GetObjectManager();
    Camera* mainCamera = Engine::GetInstance().GetCameraManager()->GetMainCamera();
    if (!mainCamera) return;

    // ���� ���� ���� �÷��̾ ���� �������� �ʾҴٸ�, "ī�޶�� 1"�� �⺻���� �����մϴ�.
    if (controlledPlayer == nullptr)
    {
        controlledPlayer = objManager->FindObjectByName("Cameraman 1");

        {
            mainCamera->SetNear(0.5f);
            glm::vec3 playerPos = controlledPlayer->transform.GetPosition();
            glm::vec3 eyeOffset = { 0.0f, 0.4f, 0.0f };
            mainCamera->SetCameraPosition(playerPos + eyeOffset);
        }

        {
            Camera* sceCamera = Engine::GetInstance().GetCameraManager()->GetCamera(1);
            sceCamera->SetNear(0.5f);
            glm::vec3 playerPos = objManager->FindObjectByName("Cameraman 2")->transform.GetPosition();
            glm::vec3 eyeOffset = { 0.0f, 0.4f, 0.0f };
            sceCamera->SetCameraPosition(playerPos + eyeOffset);
        }
    }

    // 1, 2�� Ű�� ������ �÷��̾� ����
    if (input->IsKeyPressOnce(KEYBOARDKEYS::NUMBER_1))
    {
        controlledPlayer = objManager->FindObjectByName("Cameraman 1");
        std::cout << "[Control] Cameraman 1 selected." << std::endl;
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::NUMBER_2))
    {
        controlledPlayer = objManager->FindObjectByName("Cameraman 2");
        std::cout << "[Control] Cameraman 2 selected." << std::endl;
    }

    // 3�� Ű�� 1��Ī/3��Ī ���� ��ȯ
    if (input->IsKeyPressOnce(KEYBOARDKEYS::NUMBER_3))
    {
        bool isThirdPerson = mainCamera->GetIsThirdPersonView();
        mainCamera->SetIsThirdPersonView(!isThirdPerson);
        std::cout << "[Camera] View mode changed to " << (!isThirdPerson ? "Third-Person" : "First-Person") << std::endl;
    }

    // ���� ���� �÷��̾ �ִٸ� ī�޶�� ����ȭ
    if (controlledPlayer)
    {
        // �÷��̾� ������ ī�޶� ����� ����ȭ (�� �κ��� ����)
        controlledPlayer->transform.SetRotation({ mainCamera->GetPitch() + 90.0f, -mainCamera->GetYaw() - 90.0f, 0.0f });

        if (mainCamera->GetIsThirdPersonView()) // 3��Ī ����� ��
        {
            // 1. ī�޶��� Near Clip�� �⺻������ �ǵ����ϴ�.
            mainCamera->SetNear(0.1f);
            // 2. ī�޶� �÷��̾ �ٶ󺸵��� Target ����
            mainCamera->SetTarget(controlledPlayer->transform.GetPosition());
        }
        else // 1��Ī ����� ��
        {
            // 1. ī�޶��� Near Clip�� �÷� ����� ��ü(�ڽ��� ��)�� �׸��� �ʰ� �մϴ�.
            mainCamera->SetNear(0.5f);

            // 2. ī�޶� ��ġ�� �÷��̾��� '��' ��ġ�� �����մϴ�.
            // (������Ʈ �߽ɿ��� Y������ �ణ ��, Z������ �ణ ������)
            glm::vec3 playerPos = controlledPlayer->transform.GetPosition();
            glm::vec3 eyeOffset = { 0.0f, 0.4f, 0.0f }; // �� ���� ������
            mainCamera->SetCameraPosition(playerPos + eyeOffset);
        }
    }
}