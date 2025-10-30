#pragma once

#include <vector>
#include <string>
#include "glm.hpp"
#include "gtc/quaternion.hpp"

struct aiNodeAnim;

// Ű������ ������ ����ü
struct KeyPosition { glm::vec3 position; float timeStamp; };
struct KeyRotation { glm::quat orientation; float timeStamp; };
struct KeyScale { glm::vec3 scale; float timeStamp; };

// �ϳ��� ��(Bone)�� ���� ��� Ű������ ������ �����ϴ� Ŭ����
class Bone
{
public:
    Bone(const std::string& name, int ID, const aiNodeAnim* channel);

    void Update(float animationTime);

    // Getter �Լ���
    glm::mat4 GetLocalTransform() const { return localTransform; }
    const std::string& GetBoneName() const { return name; }
    int GetBoneID() const { return id; }

private:
    glm::mat4 InterpolatePosition(float animationTime);
    glm::mat4 InterpolateRotation(float animationTime);
    glm::mat4 InterpolateScaling(float animationTime);

    int GetPositionIndex(float animationTime);
    int GetRotationIndex(float animationTime);
    int GetScaleIndex(float animationTime);
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;
    int numPositions;
    int numRotations;
    int numScales;

    glm::mat4 localTransform;
    std::string name;
    int id;
};