#include "MeshesScene.hpp"

#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "MeshRenderer.hpp"
#include "SceneManager.hpp"
#include "RenderManager.hpp"
#include "InputManager.hpp"
#include "CameraManager.hpp"
#include "Light.hpp"
#include "Model.hpp"
#include "Shader.hpp"

#include <assimp/scene.h> 

MeshesScene::MeshesScene() = default;
MeshesScene::~MeshesScene() = default;

void MeshesScene::Init()
{
    ObjectManager* objectManager = Engine::GetInstance().GetObjectManager();
    RenderManager* renderManager = Engine::GetInstance().GetRenderManager();
    renderManager->LoadShader("basic", "asset/shaders/basic.vert", "asset/shaders/basic.frag");
    renderManager->LoadShader("weight_debug", "asset/shaders/weight_debug.vert", "asset/shaders/weight_debug.frag");
    renderManager->LoadTexture("wall", "asset/wall.jpg");
    renderManager->LoadTexture("container", "asset/container.jpg");
    renderManager->LoadTexture("backpack", "asset/models/backpack/diffuse.jpg");

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

    // 구
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(2), [&](Object* object) {
        object->SetName("Sphere");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->CreateSphere();
        renderer->SetShader("basic");
        renderer->SetTexture("wall");
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

    //// 붉은색 조명
    //objectManager->AddObject<Object>();
    //objectManager->QueueObjectFunction(objectManager->FindObject(7), [&](Object* object) {
    //    object->SetName("Red Light");
    //    object->transform.SetPosition({ -2.0f, 1.0f, 2.0f });

    //    auto lightComp = object->AddComponent<Light>();
    //    lightComp->SetColor({ 1.0f, 0.0f, 0.0f });
    //    lightComp->SetType(LightType::Point);

    //    auto renderer = object->AddComponent<MeshRenderer>();
    //    renderer->CreateSphere();
    //    renderer->SetShader("basic");
    //    renderer->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
    //    object->transform.SetScale(0.2f, 0.2f, 0.2f);
    //});

    //// 푸른색 조명
    //objectManager->AddObject<Object>();
    //objectManager->QueueObjectFunction(objectManager->FindObject(8), [&](Object* object) {
    //    object->SetName("Blue Light");
    //    object->transform.SetPosition({ 2.0f, 1.0f, 2.0f });

    //    auto lightComp = object->AddComponent<Light>();
    //    lightComp->SetColor({ 0.0f, 0.0f, 1.0f });
    //    lightComp->SetType(LightType::Point);

    //    auto renderer = object->AddComponent<MeshRenderer>();
    //    renderer->CreateSphere();
    //    renderer->SetShader("basic");
    //    renderer->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f });
    //    object->transform.SetScale(0.2f, 0.2f, 0.2f);
    //});

    //// 초록색 조명
    //objectManager->AddObject<Object>();
    //objectManager->QueueObjectFunction(objectManager->FindObject(9), [&](Object* object) {
    //    object->SetName("Green Light");
    //    object->transform.SetPosition({ 0.0f, 1.0f, -2.0f });

    //    auto lightComp = object->AddComponent<Light>();
    //    lightComp->SetColor({ 0.0f, 1.0f, 0.0f });
    //    //lightComp->SetOffsetForPointL({ 0.0f, 0.0f, 2.0f });
    //    lightComp->SetType(LightType::Point);

    //    auto renderer = object->AddComponent<MeshRenderer>();
    //    renderer->CreateSphere();
    //    renderer->SetShader("basic");
    //    renderer->SetColor({ 0.0f, 1.0f, 0.0f, 1.0f });
    //    object->transform.SetScale(0.2f, 0.2f, 0.2f);
    //});

    // 직접 조명
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(7), [&](Object* object) {
        object->SetName("Direction Light");
        object->transform.SetPosition({ 0.0f, 1.0f, -2.0f });

        auto lightComp = object->AddComponent<Light>();
        lightComp->SetColor({ 1.0f, 1.0f, 1.0f });
        lightComp->SetType(LightType::Directional);
        lightComp->SetDirection({ -0.2f, -1.0f, -0.3f });
    });

    // FBX 모델
    objectManager->AddObject<Object>();
    objectManager->QueueObjectFunction(objectManager->FindObject(8), [&](Object* object) {
        object->SetName("AnimationTestObj");
        auto renderer = object->AddComponent<MeshRenderer>();
        renderer->LoadModel("asset/models/Test.fbx");

        renderer->SetShader("basic");
        // renderer->SetTexture("character_texture");
        object->transform.SetPosition(0.0f, -0.5f, -1.25f);
        object->transform.SetRotationY(180.f);
        object->transform.SetScale(0.01f, 0.01f, 0.01f);
    });


    CameraManager* cameraManager = Engine::GetInstance().GetCameraManager();
    int index = cameraManager->CreateCamera();
    Camera* camera = cameraManager->GetCamera(index);
    camera->SetCameraPosition(glm::vec3{ 0.f,0.f,-3.f });
    cameraManager->SetMainCamera(index);


    // 디버그용 셰이더 로드 (기존 basic 셰이더를 사용해도 무방)
    debugShader = renderManager->GetShader("basic");

    // 뼈 위치에 그려줄 작은 구 메쉬 생성 및 GPU 업로드
    boneMesh = std::make_unique<Mesh>();
    boneMesh->CreateDiamond();
    boneMesh->UploadToGPU();
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
    if (input->IsKeyPressOnce(KEYBOARDKEYS::NUMBER_1))
    {
        isWeightDebugMode = !isWeightDebugMode; // 모드 전환

        Object* loadedModelObject = Engine::GetInstance().GetObjectManager()->FindObjectByName("AnimationTestObj");
        if (loadedModelObject)
        {
            MeshRenderer* renderer = loadedModelObject->GetComponent<MeshRenderer>();
            if (isWeightDebugMode)
            {
                renderer->SetShader("weight_debug");
                std::cout << "[Debug] Weight Map View ON" << std::endl;
            }
            else
            {
                renderer->SetShader("basic");
                std::cout << "[Debug] Weight Map View OFF" << std::endl;
            }
        }
    }
    if (input->IsKeyPressOnce(KEYBOARDKEYS::NUMBER_2))
    {
        bDrawSkeleton = !bDrawSkeleton;
        std::cout << "[Debug] Skeleton View " << (bDrawSkeleton ? "ON" : "OFF") << std::endl;
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

inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

void MeshesScene::PostRender(Camera* camera)
{
    // 뼈 그리기가 비활성화 상태이거나, 유효한 카메라가 없으면 즉시 종료
    if (!bDrawSkeleton || !camera) return;

    Object* character = Engine::GetInstance().GetObjectManager()->FindObjectByName("AnimationTestObj");
    Model* model = character ? character->GetComponent<MeshRenderer>()->GetModel() : nullptr;
    const aiScene* scene = model ? model->GetAssimpScene() : nullptr;

    if (character && model && scene)
    {
        glDisable(GL_DEPTH_TEST);
        // 뼈 계층 구조의 시작점부터 그리기를 시작하며, 현재 카메라 정보를 전달
        DrawBoneHierarchy(scene->mRootNode, character->transform.GetModelMatrix(), camera);
        glEnable(GL_DEPTH_TEST);
    }
}

void MeshesScene::DrawBoneHierarchy(const aiNode* node, const glm::mat4& parentTransform, Camera* camera)
{
    // 현재 뼈의 월드 변환 계산
    glm::mat4 nodeTransform = ConvertMatrixToGLMFormat(node->mTransformation);
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    // 현재 뼈의 위치 그리기
    if (debugShader && boneMesh)
    {
        debugShader->Bind();

        debugShader->SetUniformMat4f("view", camera->GetViewMatrix());
        debugShader->SetUniformMat4f("projection", camera->GetProjectionMatrix());

        debugShader->SetUniform1i("useTexture", 0);
        debugShader->SetUniformVec4("color", { 0.0f, 1.0f, 0.0f, 1.0f });

        // 뼈의 최종 월드 위치에 뼈 를 위치시키고 크기를 조절합니다.
        glm::mat4 boneModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(globalTransform[3]));
        boneModelMatrix = glm::scale(boneModelMatrix, glm::vec3(0.03f));
        debugShader->SetUniformMat4f("model", boneModelMatrix);

        VertexArray* va = boneMesh->GetVertexArray();
        if (va)
        {
            va->Bind();
            glDrawElements(static_cast<GLenum>(boneMesh->GetPrimitivePattern()), boneMesh->GetIndicesCount(), GL_UNSIGNED_INT, 0);
            va->UnBind();
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        DrawBoneHierarchy(node->mChildren[i], globalTransform, camera);
    }
}

void MeshesScene::RenderImGui()
{
    Engine::GetInstance().GetObjectManager()->ObjectControllerForImgui();
}