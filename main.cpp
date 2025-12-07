#include "Engine.hpp"
#include "SceneManager.hpp"
#include "MeshesScene.hpp"
#include "PBRScene.hpp"
#include "GameScene.hpp"

#pragma comment(lib, "opengl32.lib")

int main(int argc, char* argv[])
{
    Engine::GetInstance().Init(1920, 1080);
    SceneManager* sceneManager = Engine::GetInstance().GetSceneManager();

    // 씬 생성
    sceneManager->AddScene(SceneTag::MESHES, new MeshesScene());
    sceneManager->AddScene(SceneTag::PBR, new PBRScene());
    sceneManager->AddScene(SceneTag::GAME, new GameScene());
    // 시작 씬 설정
    sceneManager->Init(SceneTag::PBR);

    // 엔진 실행
    Engine::GetInstance().Run();
    Engine::GetInstance().Shutdown();

    return 0;
}
