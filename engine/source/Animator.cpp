#include "Animator.hpp"
#include "Animation.hpp"
#include "Object.hpp"

#include<iostream>

Animator::Animator(std::unique_ptr<Animation> animation)
    : Component(ComponentTypes::ANIMATOR),
    currentAnimation(std::move(animation)),
    currentTime(0.0f)
{
    finalBoneMatrices.reserve(100);
    for (int i = 0; i < 100; i++)
        finalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::Init() {}
void Animator::End() {}

void Animator::Update(float dt)
{
    if (currentAnimation)
    {
        globalBoneTransforms.clear();

        currentTime += currentAnimation->GetTicksPerSecond() * dt;
        currentTime = fmod(currentTime, currentAnimation->GetDuration());

        glm::mat4 rootTransform = GetOwner()->transform.GetModelMatrix();
        CalculateBoneTransform(&currentAnimation->GetRootNode(), rootTransform);
    }
}

const std::vector<glm::mat4>& Animator::GetFinalBoneMatrices() const
{
    return finalBoneMatrices;
}

void Animator::CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone* bone = currentAnimation->FindBone(nodeName);

    if (bone) {
        bone->Update(currentTime);
        nodeTransform = bone->GetLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;
    globalBoneTransforms[nodeName] = globalTransformation;

    const auto& boneInfoMap = currentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end())
    {
        int index = boneInfoMap.at(nodeName).id;
        glm::mat4 offset = boneInfoMap.at(nodeName).offsetMatrix;
        finalBoneMatrices[index] = globalTransformation * offset;
    }

    for (int i = 0; i < node->childrenCount; i++)
        CalculateBoneTransform(&node->children[i], globalTransformation);
}