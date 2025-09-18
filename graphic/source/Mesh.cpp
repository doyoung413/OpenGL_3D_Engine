#include "Mesh.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitivePattern pattern)
    : vertices(vertices), indices(indices), primitivePattern(pattern)
{
}

void Mesh::UploadToGPU()
{
    VertexBuffer vb(std::span{ vertices }, vertices.size() * sizeof(Vertex));
    IndexBuffer ib(std::span{ indices });

    // VertexArray를 생성
    vertexArray = std::make_unique<VertexArray>();

    // VertexArray에 VertexBuffer와 그 데이터 레이아웃을 전달
    vertexArray->AddVertexBuffer(std::move(vb), {
        // layout 0: Position (vec3)
        {.dimension = 3, .layoutLocation = 0, .offset = (GLintptr)offsetof(Vertex, position), .stride = sizeof(Vertex) },
        // layout 1: Color (vec3)
        {.dimension = 3, .layoutLocation = 1, .offset = (GLintptr)offsetof(Vertex, color), .stride = sizeof(Vertex) },
        // layout 2: Texture Coordinate (vec2)
        {.dimension = 2, .layoutLocation = 2, .offset = (GLintptr)offsetof(Vertex, texCoord), .stride = sizeof(Vertex) }
        // -------------------
        });

    // VertexArray에 IndexBuffer를 전달
    vertexArray->AddIndexBuffer(std::move(ib));
}

void Mesh::CreatePlane()
{
    vertices.clear();
    indices.clear();

    vertices = {
        { {-0.5f, 0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} }, // 좌측 하단
        { { 0.5f, 0.0f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} }, // 우측 하단
        { { 0.5f, 0.0f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} }, // 우측 상단
        { {-0.5f, 0.0f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }  // 좌측 상단
    };

    indices = {
        0, 1, 2,  // 첫 번째 삼각형
        2, 3, 0   // 두 번째 삼각형
    };

    primitivePattern = PrimitivePattern::Triangles;
}

void Mesh::CreateCube()
{
    vertices.clear();
    indices.clear();

    vertices = {
        // 위치,                   색상(흰색),         텍스처 좌표
        // 뒷면
        { {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        // 앞면
        { {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        // 왼쪽 면
        { {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        // 오른쪽 면
        { { 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        // 아랫면
        { {-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        // 윗면
        { {-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }
    };

    indices = {
        0, 1, 2,   2, 3, 0,    4, 5, 6,   6, 7, 4,
        8, 9, 10,  10, 11, 8,   12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16, 20, 21, 22, 22, 23, 20
    };
    primitivePattern = PrimitivePattern::Triangles;
}

void Mesh::CreateSphere()
{
    // 기본값 설정
    const float radius = 0.5f;
    const int sectorCount = 36;
    const int stackCount = 18;

    vertices.clear();
    indices.clear();

    float x, y, z, xy;                             
    float u, v;                                    

    float sectorStep = 2.0f * static_cast<float>(M_PI) / sectorCount;
    float stackStep = static_cast<float>(M_PI) / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = static_cast<float>(M_PI) / 2.0f - i * stackStep;
        xy = radius * cosf(stackAngle);
        y = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;

            // 정점 위치 계산
            x = xy * cosf(sectorAngle);
            z = xy * sinf(sectorAngle);

            // 텍스처 좌표 계산
            u = (float)j / sectorCount;
            v = (float)i / stackCount;

            vertices.push_back({ {x, y, z}, {1.0f, 1.0f, 1.0f}, {u, v} });
        }
    }

    // 사각형 메시를 두 개의 삼각형
    unsigned int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // 현재 스택의 시작 인덱스
        k2 = k1 + sectorCount + 1;      // 다음 스택의 시작 인덱스

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2개의 삼각형으로 하나의 사각형 면
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    primitivePattern = PrimitivePattern::Triangles;
}

void Mesh::CreateDiamond()
{
    const float size = 1.0f;

    vertices.clear();
    indices.clear();

    float halfSize = size * 0.5f;

    vertices = {
        // 6개의 꼭짓점
        { { 0.0f,  halfSize,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f} }, // 0: Top
        { { 0.0f, -halfSize,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.0f} }, // 1: Bottom
        { {-halfSize,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.5f} }, // 2: Left
        { { halfSize,  0.0f,  0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.5f} }, // 3: Right
        { { 0.0f,  0.0f,  halfSize}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f} }, // 4: Front
        { { 0.0f,  0.0f, -halfSize}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f} }  // 5: Back
    };

    indices = {
        // 위쪽 4개 삼각형
        0, 4, 3,   0, 3, 5,   0, 5, 2,   0, 2, 4,
        // 아래쪽 4개 삼각형
        1, 3, 4,   1, 5, 3,   1, 2, 5,   1, 4, 2
    };

    primitivePattern = PrimitivePattern::Triangles;
}

void Mesh::CreateCylinder()
{
    const float radius = 0.5f;
    const float height = 1.0f;
    const int segments = 32;

    vertices.clear();
    indices.clear();

    float halfHeight = height / 2.0f;

    // 위 
    vertices.push_back({ {0.0f, halfHeight, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f} }); // Top center
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back({ {x, halfHeight, z}, {1.0f, 1.0f, 1.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f} });
    }
    // 아래
    vertices.push_back({ {0.0f, -halfHeight, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f} }); // Bottom center
    int bottomCenterIndex = static_cast<unsigned int>(vertices.size()) - 1;
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back({ {x, -halfHeight, z}, {1.0f, 1.0f, 1.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f} });
    }
    // 옆면
    int sideStartIndex = static_cast<unsigned int>(vertices.size());
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float u = (float)i / segments;
        vertices.push_back({ {x, halfHeight, z}, {1.0f, 1.0f, 1.0f}, {u, 1.0f} }); // Top vertex
        vertices.push_back({ {x, -halfHeight, z}, {1.0f, 1.0f, 1.0f}, {u, 0.0f} }); // Bottom vertex
    }

    // 위
    for (int i = 0; i < segments; i++) {
        indices.push_back(0);
        indices.push_back(i + 2);
        indices.push_back(i + 1);
    }
    // 아래껑
    for (int i = 0; i < segments; i++) {
        indices.push_back(bottomCenterIndex);
        indices.push_back(bottomCenterIndex + i + 1);
        indices.push_back(bottomCenterIndex + i + 2);
    }
    // 옆면
    for (int i = 0; i < segments; i++) {
        int current = sideStartIndex + i * 2;
        indices.push_back(current);
        indices.push_back(current + 1);
        indices.push_back(current + 3);

        indices.push_back(current);
        indices.push_back(current + 3);
        indices.push_back(current + 2);
    }

    primitivePattern = PrimitivePattern::Triangles;
}

void Mesh::CreateCapsule()
{
    const float radius = 0.5f;
    const float height = 1.0f;
    const int segments = 24;  
    const int rings = 12;     

    vertices.clear();
    indices.clear();

    float cylinderHeight = height - 2 * radius;
    float halfCylinderHeight = cylinderHeight / 2.0f;

    // 위쪽 반구
    for (int i = 0; i <= rings / 2; i++) {
        float phi = M_PI * static_cast<float>(i) / rings;
        for (int j = 0; j <= segments; j++) {
            float theta = 2.0f * M_PI * static_cast<float>(j) / segments;
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            vertices.push_back({ {x, y + halfCylinderHeight, z}, {1.0f, 1.0f, 1.0f}, {(float)j / segments, 1.0f - (float)i / rings} });
        }
    }

    // 아래쪽 반구
    for (int i = rings / 2; i <= rings; i++) {
        float phi = M_PI * static_cast<float>(i) / rings;
        for (int j = 0; j <= segments; j++) {
            float theta = 2.0f * M_PI * static_cast<float>(j) / segments;
            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);
            vertices.push_back({ {x, y - halfCylinderHeight, z}, {1.0f, 1.0f, 1.0f}, {(float)j / segments, 1.0f - (float)i / rings} });
        }
    }

    for (int i = 0; i < rings; i++) {
        for (int j = 0; j < segments; j++) {
            int first = i * (segments + 1) + j;
            int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    primitivePattern = PrimitivePattern::Triangles;
}

void Mesh::CreateFromData(const std::vector<Vertex>& vertices_, const std::vector<unsigned int>& indices_, PrimitivePattern pattern_)
{
    this->vertices.clear();
    this->indices.clear();

    this->vertices = vertices_;
    this->indices = indices_;
    this->primitivePattern = pattern_;
}
