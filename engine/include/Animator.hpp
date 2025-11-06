#pragma once

#include "Component.hpp"
#include "glm.hpp"
#include "gtc/quaternion.hpp"
#include <vector>
#include <memory> 
#include <map>
#include <string>

class Animation;
struct AssimpNodeData;

enum class PlaybackState
{
    Playing,
    Paused,
    Stopped
};

struct RootMotionBakeOptions
{
    bool bakePositionX = false;
    bool bakePositionY = true;
    bool bakePositionZ = false;
    bool bakeRotation = true;
};

class Animator : public Component
{
public:
    Animator();

    void Init() override;
    void Update(float dt) override;
    void End() override;

    void PlayAnimation(Animation* newAnimation, bool isLoop = true, float speed = 1.f, float blendDuration = 0.25f);

    // 재생 제어
    void Play();
    void Pause();
    void Stop();

    // Getter 함수들
    Animation* GetCurrentAnimation() const { return currentAnimation; }
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const;
    const std::map<std::string, glm::mat4>& GetGlobalBoneTransforms() const { return globalBoneTransforms; }
    float GetCurrentTime() const;
    float GetDuration() const;
    float GetSpeed() const { return animationSpeed; }
    const RootMotionBakeOptions& GetBakeOptions() const { return bakeOptions; }
    PlaybackState GetPlaybackState() const { return playbackState; }
    bool IsFinished() const { return justLooped; }
    bool IsLooping() const { return isLooping; }
    bool IsRootMotionEnabled() const { return enableRootMotion; }
    const std::string& GetRootBoneName() const { return rootBoneName; }
    bool IsScrubbing() const { return isScrubbing; }

    // Setter 함수들
    void SetLooping(bool loop);
    void SetCurrentTime(float time);
    void SetSpeed(float speed) { animationSpeed = std::max(0.0f, speed); }
    void SetEnableRootMotion(bool enabled) { enableRootMotion = enabled; }
    void SetBakeOptions(const RootMotionBakeOptions& options) { bakeOptions = options; }
    void SetRootBoneName(const std::string& name) { rootBoneName = name; }
    void SetIsScrubbing(bool scrubbing) { isScrubbing = scrubbing; }

    // 루트 모션 위치를 절대 시간 기준으로 즉시 업데이트하는 함수
    void UpdateRootMotionTransformToTime(float time);

private:
    glm::mat4 CalculateAbsoluteRootMotion(Animation* anim, float time, glm::mat4 startTransform);

    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

    // 스키닝 및 뼈 계층 구조 관련
    std::vector<glm::mat4> finalBoneMatrices;
    std::map<std::string, glm::mat4> globalBoneTransforms;

    // 현재 애니메이션 상태
    Animation* currentAnimation = nullptr;
    PlaybackState playbackState = PlaybackState::Stopped;
    float currentTime = 0.0f;
    float animationSpeed = 1.0f;
    bool isLooping = true;
    bool justLooped = false;

    // 블렌딩 관련
    Animation* previousAnimation = nullptr;
    float previousTime = 0.0f;
    float blendFactor = 1.0f;
    float currentBlendDuration = 0.0f;

    // 루트 모션 관련
    bool enableRootMotion = false;
    bool isScrubbing = false; // UI 슬라이더가 조작 중인지 여부
    std::string rootBoneName = "";
    RootMotionBakeOptions bakeOptions;
    glm::mat4 rootMotionStartTransform = glm::mat4(1.0f); 
    glm::mat4 previousRootMotionStartTransform = glm::mat4(1.0f);
};