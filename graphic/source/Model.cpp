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
    //��� ������(Dangling Pointer) ���� �ذ�
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

    // ��ġ, ����, UV �� �⺻ ���� ������ ����
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        SetVertexBoneDataToDefault(vertex); // �� �����͸� �⺻������ �ʱ�ȭ
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

    // �ε��� ������ ����
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // �� ����ġ ������ ���� (�ٽ�)
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

        // �� ���� ó�� �߰ߵ� ���̶��, map�� ���� ���

        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
        {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offsetMatrix = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter;
            m_BoneCounter++;
        }
        else // �̹� ��ϵ� ����� ID�� ������
        {
            boneID = m_BoneInfoMap[boneName].id;
        }

        // �� ���� � ����(vertex)�� �󸶳� ������ �ִ���(weight) ������ ��ȸ
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            // �ش� ������ ����ִ� boneIDs/weights �迭 ������ ã�� ������ ���
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