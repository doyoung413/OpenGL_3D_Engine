#pragma once

#include "Model.hpp"
#include "Bone.hpp"
#include <assimp/scene.h>
#include <map>
#include <vector>

struct AssimpNodeData
{
    glm::mat4 transformation = glm::mat4(1.0f);
    std::string name = "";
    std::vector<AssimpNodeData> children;
};

class Animation
{
public:
    Animation(const std::string& animationPath, Model* model);
    ~Animation() = default;

    Bone* FindBone(const std::string& name);

    float GetTicksPerSecond() const { return ticksPerSecond; }
    float GetDuration() const { return duration; }
    const AssimpNodeData& GetRootNode() const { return rootNode; }
    const std::map<std::string, BoneInfo>& GetBoneIDMap() const { return boneInfoMap; }
    const std::vector<Bone>& GetBones() const { return bones; }
private:
    void ReadMissingBones(const aiAnimation* animation, Model& model);
    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src);
    glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from);

    float duration;
    float ticksPerSecond;
    std::vector<Bone> bones;
    AssimpNodeData rootNode;
    std::map<std::string, BoneInfo> boneInfoMap;
};