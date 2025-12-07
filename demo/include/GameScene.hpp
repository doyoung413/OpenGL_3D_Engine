#pragma once
#include "Scene.hpp"
#include <memory>
#include "Skybox.hpp"

class Object;

class GameScene : public Scene
{
public:
    GameScene();
    virtual ~GameScene();

    void Init() override;
    void Update(float dt) override;
    void Restart() override;
    void End() override;
    void PostRender(Camera* camera) override;
    void RenderImGui() override;

private:
    // 플레이어 조작 처리 함수
    void HandlePlayerInput(float dt);
    void UpdateSmoothRotation(const glm::vec3& inputDir, float dt);

    void CreateCoin(glm::vec3 position); 
    void CheckCoinCollisions();

    std::unique_ptr<Skybox> skybox;

    // 플레이어 오브젝트 포인터 (조작을 위해 저장)
    Object* playerObject = nullptr;

    // 게임 설정 변수
    float maxMoveSpeed = 2.5f;      // 최대 이동 속도
    float currentSpeed = 0.0f;      // 현재 속도 (가속도 적용용)
    float acceleration = 5.0f;      // 가속도 (클수록 빨리 빨라짐)
    float deceleration = 5.0f;      // 감속도 (클수록 빨리 멈춤)

    float rotationSpeed = 10.0f;    // 회전 속도 (Slerp 보간 속도)
    
    // 게임 데이터
    std::vector<Object*> coins; // 활성화된 코인 목록
    int score = 0;              // 현재 점수
    int totalCoins = 0;         // 전체 코인 수
};