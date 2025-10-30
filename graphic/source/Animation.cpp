#include "Animation.hpp"
#include <assimp/Importer.hpp>
#include <cassert>

Animation::Animation(const std::string& animationPath, Model* model)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
    assert(scene && scene->mRootNode);

    auto animation = scene->mAnimations[0];

    duration = static_cast<float>(animation->mDuration);
    ticksPerSecond = static_cast<float>(animation->mTicksPerSecond);
    if (ticksPerSecond == 0.0f) {
        ticksPerSecond = 25.0f;
    }

    ReadHierarchyData(rootNode, scene->mRootNode);
    ReadMissingBones(animation, *model);
}

Bone* Animation::FindBone(const std::string& name)
{
    auto iter = std::find_if(bones.begin(), bones.end(),
        [&](const Bone& bone) { return bone.GetBoneName() == name; }
    );
    if (iter == bones.end()) return nullptr;
    else return &(*iter);
}

void Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
{
    int size = animation->mNumChannels;
    auto& modelBoneInfoMap = model.GetBoneInfoMap();
    int boneCount = model.GetBoneCount(); // Model의 뼈 개수만 복사
    boneInfoMap = modelBoneInfoMap;       // Animation의 내부 map에 Model의 map을 복사

    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            // Model을 바꾸는 대신, Animation의 map에만 새 뼈 정보를 추가
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
        }
        bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
    }
}


void Animation::ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
{
    assert(src);
    dest.name = src->mName.data;
    dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
    dest.childrenCount = src->mNumChildren;

    for (unsigned int i = 0; i < src->mNumChildren; i++)
    {
        AssimpNodeData newData;
        ReadHierarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}

glm::mat4 Animation::ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}