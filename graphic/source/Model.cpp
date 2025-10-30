#define GLM_ENABLE_EXPERIMENTAL

#include "Model.hpp"

#include "gtc/type_ptr.hpp"
#include "gtx/quaternion.hpp"
#include <iostream>

Model::Model(const std::string& path)
{
    LoadModel(path);
}

void Model::LoadModel(const std::string& path)
{
    //허상 포인터(Dangling Pointer) 문제 해결
    scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(ProcessMesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        ProcessNode(node->mChildren[i], scene);
    }
}

std::shared_ptr<Mesh> Model::ProcessMesh(aiMesh* mesh, const aiScene* /*scene*/)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // 위치, 법선, UV 등 기본 정점 데이터 추출
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        SetVertexBoneDataToDefault(vertex); // 뼈 데이터를 기본값으로 초기화
        vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        if (mesh->HasNormals()) {
            vertex.normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        }
        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
        }
        else {
            vertex.texCoord = { 0.0f, 0.0f };
        }
        vertex.color = { 1.0f, 1.0f, 1.0f };
        vertices.push_back(vertex);
    }

    // 인덱스 데이터 추출
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // 뼈 가중치 데이터 추출 (핵심)
    ExtractBoneWeightForVertices(vertices, mesh, nullptr);

    auto newMesh = std::make_shared<Mesh>(vertices, indices, PrimitivePattern::Triangles);
    return newMesh;
}

void Model::SetVertexBoneDataToDefault(Vertex & vertex)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        vertex.boneIDs[i] = -1;
        vertex.weights[i] = 0.0f;
    }
}


inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
    glm::mat4 to;
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

void Model::ExtractBoneWeightForVertices(std::vector<Vertex>&vertices, aiMesh * mesh, const aiScene * /*scene*/)
{
    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

        // 이 뼈가 처음 발견된 것이라면, map에 새로 등록

        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offsetMatrix = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter;
            m_BoneCounter++;
        }
        else // 이미 등록된 뼈라면 ID를 가져옴
        {
            boneID = m_BoneInfoMap[boneName].id;
        }

        // 각 뼈가 어떤 정점(vertex)에 얼마나 영향을 주는지(weight) 정보를 순회
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            // 해당 정점의 비어있는 boneIDs/weights 배열 슬롯을 찾아 정보를 등록
            for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
            {
                if (vertices[vertexId].boneIDs[i] < 0)
                {
                    vertices[vertexId].weights[i] = weight;
                    vertices[vertexId].boneIDs[i] = boneID;
                    break;
                }
            }
        }
    }
}