#define GLM_ENABLE_EXPERIMENTAL

#include "Bone.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtx/quaternion.hpp"

#include <assimp/anim.h>

Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
    : name(name), id(ID), localTransform(1.0f)
{
    numPositions = static_cast<int>(channel->mNumPositionKeys);
    for (int i = 0; i < numPositions; ++i) {
        aiVector3D aiPosition = channel->mPositionKeys[i].mValue;
        float timeStamp = channel->mPositionKeys[i].mTime;
        positions.push_back({ glm::vec3(aiPosition.x, aiPosition.y, aiPosition.z), timeStamp });
    }

    numRotations = static_cast<int>(channel->mNumRotationKeys);
    for (int i = 0; i < numRotations; ++i) {
        aiQuaternion aiOrientation = channel->mRotationKeys[i].mValue;
        float timeStamp = channel->mRotationKeys[i].mTime;
        rotations.push_back({ glm::quat(aiOrientation.w, aiOrientation.x, aiOrientation.y, aiOrientation.z), timeStamp });
    }

    numScales = static_cast<int>(channel->mNumScalingKeys);
    for (int i = 0; i < numScales; ++i) {
        aiVector3D aiScale = channel->mScalingKeys[i].mValue;
        float timeStamp = channel->mScalingKeys[i].mTime;
        scales.push_back({ glm::vec3(aiScale.x, aiScale.y, aiScale.z), timeStamp });
    }
}

void Bone::Update(float animationTime)
{
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);
    localTransform = translation * rotation * scale;
}

glm::mat4 Bone::InterpolatePosition(float animationTime)
{
    if (numPositions == 1)
        return glm::translate(glm::mat4(1.0f), positions[0].position);

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(positions[p0Index].timeStamp, positions[p1Index].timeStamp, animationTime);
    glm::vec3 finalPosition = glm::mix(positions[p0Index].position, positions[p1Index].position, scaleFactor);
    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::InterpolateRotation(float animationTime)
{
    if (numRotations == 1) {
        auto rotation = glm::normalize(rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(rotations[p0Index].timeStamp, rotations[p1Index].timeStamp, animationTime);
    glm::quat finalRotation = glm::slerp(rotations[p0Index].orientation, rotations[p1Index].orientation, scaleFactor);
    finalRotation = glm::normalize(finalRotation);
    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::InterpolateScaling(float animationTime)
{
    if (numScales == 1)
        return glm::scale(glm::mat4(1.0f), scales[0].scale);

    int p0Index = GetScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(scales[p0Index].timeStamp, scales[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(scales[p0Index].scale, scales[p1Index].scale, scaleFactor);
    return glm::scale(glm::mat4(1.0f), finalScale);
}

int Bone::GetPositionIndex(float animationTime)
{
    for (int index = 0; index < numPositions - 1; ++index) {
        if (animationTime < positions[index + 1].timeStamp)
            return index;
    }
    // 기본적으로 마지막에서 두 번째 인덱스를 반환하지만, 실제로는 루프 내에서 반환됨
    return 0;
}

int Bone::GetRotationIndex(float animationTime)
{
    for (int index = 0; index < numRotations - 1; ++index) {
        if (animationTime < rotations[index + 1].timeStamp)
            return index;
    }
    return 0;
}

int Bone::GetScaleIndex(float animationTime)
{
    for (int index = 0; index < numScales - 1; ++index) {
        if (animationTime < scales[index + 1].timeStamp)
            return index;
    }
    return 0;
}

float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float scaleFactor = 0.0f;
    float midWayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    // 0으로 나누는 것을 방지
    if (framesDiff > 0.0f) {
        scaleFactor = midWayLength / framesDiff;
    }
    return scaleFactor;
}