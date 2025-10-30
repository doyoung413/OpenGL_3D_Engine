#pragma once

#include "Component.hpp"
#include "glm.hpp"
#include <vector>
#include <memory> 
#include <map>
#include <string>

class Animation;
struct AssimpNodeData;

class Animator : public Component
{
public:
    Animator(std::unique_ptr<Animation> animation);

    void Init() override;
    void Update(float dt) override;
    void End() override;

    Animation* GetCurrentAnimation() const { return currentAnimation.get(); }
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const;
    const std::map<std::string, glm::mat4>& GetGlobalBoneTransforms() const { return globalBoneTransforms; }
private:
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);

    std::vector<glm::mat4> finalBoneMatrices;
    std::unique_ptr<Animation> currentAnimation;
    std::map<std::string, glm::mat4> globalBoneTransforms;
    float currentTime;
};