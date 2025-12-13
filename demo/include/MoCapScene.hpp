#pragma once
#include "Scene.hpp"
#include <vector>
#include <memory>

#include "glm.hpp"

class Object;
class MotionCaptureSystem;
struct PoseLandmark;
class MoCapScene : public Scene
{
public:
    MoCapScene();
    virtual ~MoCapScene();

    void Init() override;
    void Update(float dt) override;
    void Restart() override;
    void End() override;
    void PostRender(Camera* camera) override;
    void RenderImGui() override;

private:
    void CreateJoints();
    void CreateBones();

    // AI 데이터로 위치 동기화 함수
    void SyncJoints();
    void SyncBones();
    void ResetSkeletonState();

    glm::quat GetRotationBetweenVectors(glm::vec3 sourceVector, glm::vec3 targetVector);

    std::unique_ptr<MotionCaptureSystem> mocapSystem;

    // 33개 관절과 뼈를 시각화할 구체 오브젝트 목록
    std::vector<Object*> jointSpheres; 
    std::vector<Object*> boneCylinders;

    // 디버그 텍스처용 ID
    unsigned int debugTextureID = 0;

    // BlazePose 33개 키포인트 기준
    const std::vector<std::pair<int, int>> boneConnections = {
        // 몸통
        {11, 12}, {11, 23}, {12, 24}, {23, 24},
        // 오른팔
        {12, 14}, {14, 16},
        // 왼팔
        {11, 13}, {13, 15},
        // 오른다리
        {24, 26}, {26, 28}, {28, 30}, {30, 32}, {28, 32},
        // 왼다리
        {23, 25}, {25, 27}, {27, 29}, {29, 31}, {27, 31}
    };
};