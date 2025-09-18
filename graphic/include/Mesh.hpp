#pragma once
#include <vector>
#include <memory>
#include <glm.hpp>
#include "VertexArray.hpp"

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
    glm::vec3 color;
    glm::vec2 texCoord;
};

class Mesh
{
public:
    Mesh() = default;
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitivePattern pattern = PrimitivePattern::Triangles);

    void CreatePlane();
    void CreateCube();
    void CreateSphere();
    void CreateDiamond();
    void CreateCylinder();
    void CreateCapsule();
    void CreateFromData(const std::vector<Vertex>& vertices_, const std::vector<unsigned int>& indices_, PrimitivePattern pattern_);

    void UploadToGPU();


    VertexArray* GetVertexArray() const { return vertexArray.get(); }
    PrimitivePattern GetPrimitivePattern() const { return primitivePattern; }

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    PrimitivePattern primitivePattern = PrimitivePattern::Triangles;

    std::unique_ptr<VertexArray> vertexArray;
};
