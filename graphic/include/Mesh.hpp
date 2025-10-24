#pragma once
#include <vector>
#include <memory>
#include <glm.hpp>
#include "VertexArray.hpp"

#define MAX_BONE_INFLUENCE 4 // 각 정점은 최대 4개의 뼈에 영향을 받을 수 있음

enum class PrimitivePattern
{
    Lines = 0x0001,       
    LineStrip = 0x0003,   
    LineLoop = 0x0002,    
    Triangles = 0x0004,   
    TriangleStrip = 0x0005,
    TriangleFan = 0x0006
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 texCoord;

    // 애니메이션 데이터
    int boneIDs[MAX_BONE_INFLUENCE];   // 영향을 주는 뼈의 ID
    float weights[MAX_BONE_INFLUENCE]; // 각 뼈로부터 받는 영향(가중치)
};

class Mesh
{
public:
    Mesh() = default;
    ~Mesh() = default;
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitivePattern pattern = PrimitivePattern::Triangles);

    void CreatePlane();
    void CreateCube();
    void CreateSphere();
    void CreateDiamond();
    void CreateCylinder();
    void CreateCapsule();
    void CreateCone();

    void UploadToGPU();

    VertexArray* GetVertexArray() const { return vertexArray.get(); }
    PrimitivePattern GetPrimitivePattern() const { return primitivePattern; }
    GLsizei GetIndicesCount() const { return static_cast<GLsizei>(indices.size()); }
private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    PrimitivePattern primitivePattern = PrimitivePattern::Triangles;

    std::unique_ptr<VertexArray> vertexArray;
};
