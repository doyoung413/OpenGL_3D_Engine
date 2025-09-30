#include "MeshesScene.hpp"
#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "MeshRenderer.hpp"
#include "SceneManager.hpp"
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
    renderManager->LoadTexture("backpack", "asset/models/backpack/diffuse.jpg");

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
        // �� ������ ����(normal) �����͸� �߰��մϴ�.
        // ��ġ,                     ����(���� ��),         ����,                   �ؽ�ó ��ǥ
        Vertex{ {-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, -0.5f}, {0.2f, 0.3f, 0.8f}, {0.0f, 0.0f} }, // 0
        Vertex{ { 0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, {0.2f, 0.3f, 0.8f}, {1.0f, 0.0f} }, // 1
        Vertex{ { 0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, {0.2f, 0.3f, 0.8f}, {1.0f, 1.0f} }, // 2
        Vertex{ {-0.5f, -0.5f,  0.5f}, {-0.5f, -0.5f,  0.5f}, {0.2f, 0.3f, 0.8f}, {0.0f, 1.0f} }, // 3
        Vertex{ { 0.0f,  0.5f,  0.0f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f, 0.2f}, {0.5f, 0.5f} }  // 4
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

    //�� �ҷ�����
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

    CameraManager* cameraManager = Engine::GetInstance().GetCameraManager();
    int index = cameraManager->CreateCamera();
    Camera* camera = cameraManager->GetCamera(index);
    camera->SetCameraPosition(glm::vec3{ 0.f,0.f,-3.f });
    cameraManager->SetMainCamera(index);
}

void MeshesScene::Update(float dt)
{
    HandleInputTests();
    HandleCameraInput(dt);
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

    if (input->IsKeyPressOnce(KEYBOARDKEYS::F11))
    {
        Engine::GetInstance().ToggleFullscreen();
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::ESCAPE))
    {
        Engine::GetInstance().GetSceneManager()->ChangeState(SceneState::SHUTDOWN);
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
    const float cameraSpeed = 0.5f * dt;

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

    // ��� ���콺 ��尡 �������� ���� ���� ����
    if (input->GetRelativeMouseMode())
    {
        glm::vec2 mouseDelta = input->GetRelativeMouseMotion();
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
        {
            mainCamera->UpdateCameraDirection(mouseDelta);
        }
    }
}