#include "AnimationStateMachine.hpp"
#include "Object.hpp"
#include "Animator.hpp"
#include "Animation.hpp"
#include "MeshRenderer.hpp" 
#include <iostream> // 디버깅용

AnimationStateMachine::AnimationStateMachine() : Component(ComponentTypes::INVALID) {}

void AnimationStateMachine::Init()
{
    // 이 FSM 컴포넌트가 부착된 오브젝트에서 Animator 컴포넌트를 찾아 포인터를 저장
    // FSM은 항상 Animator와 한 쌍으로 작동해야 함
    animator = GetOwner()->GetComponent<Animator>();
}

void AnimationStateMachine::Update(float /*dt*/)
{
    // FSM 자체의 Update는 비워둠
    // 실제 애니메이션 시간 계산 및 행렬 업데이트는 Animator가 독립적으로 수행
}

void AnimationStateMachine::End() {}

void AnimationStateMachine::AddState(const std::string& name, std::shared_ptr<Animation> anim)
{
    // 전달받은 Animation 포인터로 새로운 State를 생성하고 states map에 추가
    states.emplace(name, std::make_unique<State>(anim));
    currentState = states[name].get();
}

void AnimationStateMachine::AddState(const std::string& name, const std::string& animationFilePath)
{
    Object* owner_ = GetOwner();
    if (!owner_) return;
    MeshRenderer* renderer = owner_->GetComponent<MeshRenderer>();
    if (!renderer) return;
    Model* model = renderer->GetModel();
    if (!model) return;

    // Animation을 shared_ptr로 생성
    auto animation = std::make_shared<Animation>(animationFilePath, model);
    // 생성된 Animation을 공동 소유하는 State를 생성하여 map에 추가
    states.emplace(name, std::make_unique<State>(animation));
}

void AnimationStateMachine::ChangeState(const std::string& name, bool isLoop, float speed, float blendDuration)
{
    if (states.count(name))
    {
        State* nextState = states[name].get();

        if (currentState != nextState)
        {
            currentState = nextState;

            if (animator)
            {
                animator->PlayAnimation(currentState->animation.get(), isLoop, speed, blendDuration);
                std::cout << "Animation state changed to: " << name << std::endl;
            }
        }
    }
}

std::vector<std::string> AnimationStateMachine::GetStateNames() const
{
    std::vector<std::string> names;
    for (const auto& pair : states)
    {
        names.push_back(pair.first);
    }
    return names;
}

std::string AnimationStateMachine::GetCurrentStateName() const
{
    if (currentState == nullptr)
    {
        return "None";
    }

    for (const auto& pair : states)
    {
        if (pair.second.get() == currentState)
        {
            return pair.first;
        }
    }
    return "Unknown";
}