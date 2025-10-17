#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include <map>
#include <vector>
#include <string>

#include "Mesh.hpp"

// 뼈의 고유 ID와 오프셋 행렬을 저장하는 구조체
struct BoneInfo
{
    int id;
    glm::mat4 offsetMatrix;
};

class Model
{
public:
    Model(const std::string& path);
    const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return meshes; }

    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }
    const aiScene* GetAssimpScene() const { return scene; }
private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::string directory;

    struct BoneInfo { int id; glm::mat4 offsetMatrix; };
    std::map<std::string, BoneInfo> m_BoneInfoMap; 
    Assimp::Importer importer;
    const aiScene* scene;
    int m_BoneCounter = 0;

    void LoadModel(const std::string& path);
    void ProcessNode(aiNode* node, const aiScene* scene);
    std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);

    void SetVertexBoneDataToDefault(Vertex& vertex);
    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
};