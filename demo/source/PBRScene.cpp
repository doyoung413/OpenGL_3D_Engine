#include "PBRScene.hpp"
#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "RenderManager.hpp"
#include "CameraManager.hpp"
#include "MeshRenderer.hpp"
#include "Light.hpp"
#include "InputManager.hpp"

PBRScene::PBRScene() {}
PBRScene::~PBRScene() {}

void PBRScene::Init()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();

    renderManager->LoadShader("pbr", "asset/shaders/pbr.vert", "asset/shaders/pbr.frag");
    renderManager->LoadShader("basic", "asset/shaders/basic.vert", "asset/shaders/basic.frag");

    skybox = std::make_unique<Skybox>();
    skybox->Init("asset/hdr/modern_evening_street_4k.hdr");

    int rows = 7;
    int cols = 7;
    float spacing = 2.5f;

    for (int row = 0; row < rows; ++row)
    {
        float metallic = (float)row / (float)(rows - 1);

        for (int col = 0; col < cols; ++col)
        {
            float roughness = glm::clamp((float)col / (float)(cols - 1), 0.05f, 1.0f);

            objectManager->AddObject<Object>();
            auto lastObjPtr = objectManager->GetObjectList().back().get();

            objectManager->QueueObjectFunction(lastObjPtr, [=](Object* obj) {
                obj->SetName("Sphere_M" + std::to_string((int)(metallic * 10)) + "_R" + std::to_string((int)(roughness * 10)));

                auto renderer = obj->AddComponent<MeshRenderer>();
                renderer->CreateSphere();
                renderer->SetShader("pbr");

                float xPos = (col - (cols / 2)) * spacing;
                float yPos = (row - (rows / 2)) * spacing;
                obj->transform.SetPosition(xPos, yPos, 0.0f);

                renderer->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
                renderer->SetMetallic(metallic);
                renderer->SetRoughness(roughness);
            });
        }
    }

    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3(10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };

    for (int i = 0; i < 4; ++i)
    {
        objectManager->AddObject<Object>();
        auto lightObj = objectManager->GetObjectList().back().get();

        objectManager->QueueObjectFunction(lightObj, [=](Object* obj) {
            obj->SetName("Light " + std::to_string(i));
            obj->transform.SetPosition(lightPositions[i]);

            auto light = obj->AddComponent<Light>();
            light->SetType(LightType::Point);
            light->SetColor({ 1.0f, 1.0f, 1.0f });
        });
    }

    CameraManager* camManager = Engine::GetInstance().GetCameraManager();
    camManager->ClearCameras();

    int camIdx = camManager->CreateCamera();
    Camera* mainCam = camManager->GetCamera(camIdx);

    mainCam->SetCameraPosition({ 0.0f, 0.0f, 25.0f });
    mainCam->SetYaw(270.f);
    camManager->SetMainCamera(camIdx);

    objectManager->Init();
}

void PBRScene::Update(float dt)
{
    if (skybox) skybox->BindIBL();
    Camera* mainCam = Engine::GetInstance().GetCameraManager()->GetMainCamera();
    InputManager* input = Engine::GetInstance().GetInputManager();

    if (mainCam && input)
    {
        float speed = 10.0f * dt;
        if (input->IsKeyPressed(KEYBOARDKEYS::W)) mainCam->MoveCameraPos(CameraMoveDir::FORWARD, speed);
        if (input->IsKeyPressed(KEYBOARDKEYS::S)) mainCam->MoveCameraPos(CameraMoveDir::BACKWARD, speed);
        if (input->IsKeyPressed(KEYBOARDKEYS::A)) mainCam->MoveCameraPos(CameraMoveDir::LEFT, speed);
        if (input->IsKeyPressed(KEYBOARDKEYS::D)) mainCam->MoveCameraPos(CameraMoveDir::RIGHT, speed);
        if (input->IsKeyPressed(KEYBOARDKEYS::SPACE)) mainCam->MoveCameraPos(CameraMoveDir::UP, speed);
        if (input->IsKeyPressed(KEYBOARDKEYS::LSHIFT)) mainCam->MoveCameraPos(CameraMoveDir::DOWN, speed);

        if (input->GetRelativeMouseMode()) mainCam->UpdateCameraDirection(input->GetRelativeMouseMotion());

        if (input->IsMouseButtonPressOnce(MOUSEBUTTON::RIGHT)) input->SetRelativeMouseMode(true);
        if (input->IsMouseButtonReleasedOnce(MOUSEBUTTON::RIGHT)) input->SetRelativeMouseMode(false);
    }
}

void PBRScene::PostRender(Camera* camera)
{
    if (skybox) skybox->Render(camera);
    Engine::GetInstance().GetObjectManager()->RenderGizmos(camera);
}

void PBRScene::Restart() {}
void PBRScene::End()
{
    Engine::GetInstance().GetObjectManager()->DestroyAllObjects();
    Engine::GetInstance().GetRenderManager()->ResetAllResources();
    Engine::GetInstance().GetCameraManager()->ClearCameras();
}

void PBRScene::RenderImGui()
{
    Engine::GetInstance().GetObjectManager()->ObjectControllerForImgui();
    Engine::GetInstance().GetCameraManager()->CameraControllerForImGui();
}