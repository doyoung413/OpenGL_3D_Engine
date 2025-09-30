#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <memory>
#include <vector>
#include <string>

#include "Mesh.hpp"

class Model
{
public:
    Model(const std::string& path);
    const std::vector<std::shared_ptr<Mesh>>& GetMeshes() const { return meshes; }

private:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::string directory;

    void LoadModel(const std::string& path);
    void ProcessNode(aiNode* node, const aiScene* scene);
    std::shared_ptr<Mesh> ProcessMesh(aiMesh* mesh, const aiScene* scene);
};