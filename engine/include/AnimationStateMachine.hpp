#pragma once

#include "Component.hpp"
#include <string>
#include <map>
#include <memory>
#include <vector>

class Animation;
class Animator;

class AnimationStateMachine : public Component
{
public:
    AnimationStateMachine();

    void Init() override;
    void Update(float dt) override;
    void End() override;

    //  외부(Scene 등)에서 이미 생성된 Animation을 공유하여 사용
    void AddState(const std::string& name, std::shared_ptr<Animation> anim);
    // 파일 경로를 받아 FSM이 직접 Animation을 생성하고 소유
    void AddState(const std::string& name, const std::string& animationFilePath);
    void ChangeState(const std::string& name, bool isLoop = true, float speed = 1.f, float blendDuration = 0.25f);


    std::vector<std::string> GetStateNames() const;
    std::string GetCurrentStateName() const;
private:
    // 하나의 상태를 표현하는 간단한 내부 클래스
    class State
    {
    public:
        State(std::shared_ptr<Animation> anim) : animation(anim) {}
        std::shared_ptr<Animation> animation;
    };

    std::map<std::string, std::unique_ptr<State>> states;
    State* currentState = nullptr;
    Animator* animator = nullptr;
};