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

    // VertexArray�� ����
    vertexArray = std::make_unique<VertexArray>();

    // VertexArray�� VertexBuffer�� �� ������ ���̾ƿ��� ����
    vertexArray->AddVertexBuffer(std::move(vb), {
            // layout 0: Position (vec3)
        {.dimension = 3, .layoutLocation = 0, .offset = (GLintptr)offsetof(Vertex, position), .stride = sizeof(Vertex) },
            // layout 1: normal (vec3)
        {.dimension = 3, .layoutLocation = 1, .offset = (GLintptr)offsetof(Vertex, normal),   .stride = sizeof(Vertex) },
            // layout 2: Color (vec3)
        {.dimension = 3, .layoutLocation = 2, .offset = (GLintptr)offsetof(Vertex, color),    .stride = sizeof(Vertex) },
            // layout 3: Texture Coordinate (vec2)
        {.dimension = 2, .layoutLocation = 3, .offset = (GLintptr)offsetof(Vertex, texCoord), .stride = sizeof(Vertex) } ,
            // layout 4 : Bone IDs
        {.dimension = 4, .layoutLocation = 4, .type = GL_INT, .offset = (GLintptr)offsetof(Vertex, boneIDs), .stride = sizeof(Vertex), .isIntegerType = true },
            // layout 5 : Weights
        {.dimension = 4, .layoutLocation = 5, .offset = (GLintptr)offsetof(Vertex, weights), .stride = sizeof(Vertex) }
        });

    // VertexArray�� IndexBuffer�� ����
    vertexArray->AddIndexBuffer(std::move(ib));
}

void Mesh::CreatePlane()
{
    vertices.clear();
    indices.clear();

    vertices = {
        // ��ġ,                   ����(��� ���� ����),   ����(���),         �ؽ�ó ��ǥ
        { {-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { { 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { { 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { {-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }
    };

    indices = {
        0, 1, 2,  // ù ��° �ﰢ��
        2, 3, 0   // �� ��° �ﰢ��
    };

    primitivePattern = PrimitivePattern::Triangles;
}

void Mesh::CreateCube()
{
    vertices.clear();
    indices.clear();

    vertices = {
        // ��ġ,                   ����,                 ����,             �ؽ�ó ��ǥ
        // �޸� (-Z)
        { {-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        // �ո� (+Z)
        { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        // ���� �� (-X)
        { {-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { {-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { {-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        // ������ �� (+X)
        { { 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f},  {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        // �Ʒ��� (-Y)
        { {-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f},{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { { 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f},{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { { 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f},{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { {-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f},{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} },
        // ���� (+Y)
        { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f} },
        { { 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f} },
        { { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f} },
        { {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f} }
    };

    indices = {
        0, 1, 2,   2, 3, 0,
        4, 5, 6,   6, 7, 4,
        8, 9, 10,  10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };
    primitivePattern = PrimitivePattern::Triangles;
}

void Mesh::CreateSphere()
{
    // �⺻�� ����
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

            // ���� ��ġ ���
            x = xy * cosf(sectorAngle);
            z = xy * sinf(sectorAngle);
            glm::vec3 pos = { x, y, z };

            // ���� ���� (���� �߽��� �����̹Ƿ�, ��ġ ���͸� ����ȭ�ϸ� ������ ��)
            glm::vec3 nrm = glm::normalize(pos);

            // �ؽ�ó ��ǥ ���
            u = (float)j / sectorCount;
            v = (float)i / stackCount;

            vertices.push_back({ pos, nrm, {1.0f, 1.0f, 1.0f}, {u, v} });
        }
    }

    // �簢�� �޽ø� �� ���� �ﰢ��
    unsigned int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // ���� ������ ���� �ε���
        k2 = k1 + sectorCount + 1;      // ���� ������ ���� �ε���

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2���� �ﰢ������ �ϳ��� �簢�� ��
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

    // 6���� ������ ���� ��ġ ����
        glm::vec3 positions[] = {
            { 0.0f,  halfSize,  0.0f}, // 0: Top
            { 0.0f, -halfSize,  0.0f}, // 1: Bottom
            {-halfSize,  0.0f,  0.0f}, // 2: Left
            { halfSize,  0.0f,  0.0f}, // 3: Right
            { 0.0f,  0.0f,  halfSize}, // 4: Front
            { 0.0f,  0.0f, -halfSize}  // 5: Back
    };

    // �ؽ�ó ��ǥ
    glm::vec2 texcoords[] = {
        {0.5f, 1.0f}, {0.5f, 0.0f}, {0.0f, 0.5f},
        {1.0f, 0.5f}, {0.5f, 0.5f}, {0.5f, 0.5f}
    };

    // 6���� ������ ���� Vertex ��ü ����
    for (int i = 0; i < 6; ++i)
    {
        vertices.push_back({
            positions[i],                           // ��ġ
            glm::normalize(positions[i]),           // ���� (��ġ ���� ����ȭ)
            {1.0f, 1.0f, 1.0f},                     // ����
            texcoords[i]                            // �ؽ�ó ��ǥ
            });
    }

    indices = {
        // ���� 4�� �ﰢ��
        0, 4, 3,   0, 3, 5,   0, 5, 2,   0, 2, 4,
        // �Ʒ��� 4�� �ﰢ��
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

    // �� 
    glm::vec3 topNormal = { 0.0f, 1.0f, 0.0f };
    vertices.push_back({ {0.0f, halfHeight, 0.0f}, topNormal, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f} }); // Top center
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back({ {x, halfHeight, z}, topNormal, {1.0f, 1.0f, 1.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f} });
    }
    // �Ʒ�
    glm::vec3 bottomNormal = { 0.0f, -1.0f, 0.0f };
    unsigned int bottomCenterIndex = vertices.size();
    vertices.push_back({ {0.0f, -halfHeight, 0.0f}, bottomNormal, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f} });
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back({ {x, -halfHeight, z}, bottomNormal, {1.0f, 1.0f, 1.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f} });
    }
    // ����
    unsigned int sideStartIndex = vertices.size();
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        float u = (float)i / segments;
        glm::vec3 sideNormal = glm::normalize(glm::vec3(x, 0.0f, z));
        vertices.push_back({ {x, halfHeight, z}, sideNormal, {1.0f, 1.0f, 1.0f}, {u, 1.0f} }); // Top vertex
        vertices.push_back({ {x, -halfHeight, z}, sideNormal, {1.0f, 1.0f, 1.0f}, {u, 0.0f} }); // Bottom vertex
    }

    // ��
    for (int i = 0; i < segments; i++) {
        indices.push_back(0);
        indices.push_back(i + 2);
        indices.push_back(i + 1);
    }
    // �Ʒ���
    for (int i = 0; i < segments; i++) {
        indices.push_back(bottomCenterIndex);
        indices.push_back(bottomCenterIndex + i + 1);
        indices.push_back(bottomCenterIndex + i + 2);
    }
    // ����
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

    float cylinderHeight = height - 2.f * radius;
    if (cylinderHeight < 0) cylinderHeight = 0;
    float halfCylinderHeight = cylinderHeight / 2.0f;

    // ���� �ݱ�
    for (int i = 0; i <= rings / 2; i++) {
        float phi = static_cast<float>(M_PI) * static_cast<float>(i) / rings;
        for (int j = 0; j <= segments; j++) {
            float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / segments;
            glm::vec3 spherePos = {
                radius * sin(phi) * cos(theta),
                radius * cos(phi),
                radius * sin(phi) * sin(theta)
            };
            vertices.push_back({
                {spherePos.x, spherePos.y + halfCylinderHeight, spherePos.z}, // ��ġ
                glm::normalize(spherePos),                                    // ����
                {1.0f, 1.0f, 1.0f},                                           // ����
                {(float)j / segments, 1.0f - (float)i / rings}                // UV
                });
        }
    }

    // �Ʒ��� �ݱ�
    for (int i = rings / 2; i <= rings; i++) {
        float phi = static_cast<float>(M_PI) * static_cast<float>(i) / rings;
        for (int j = 0; j <= segments; j++) {
            float theta = 2.0f * static_cast<float>(M_PI) * static_cast<float>(j) / segments;
            glm::vec3 spherePos = {
                radius * sin(phi) * cos(theta),
                radius * cos(phi),
                radius * sin(phi) * sin(theta)
            };
            vertices.push_back({
                {spherePos.x, spherePos.y - halfCylinderHeight, spherePos.z}, // ��ġ
                glm::normalize(spherePos),                                    // ����
                {1.0f, 1.0f, 1.0f},                                           // ����
                {(float)j / segments, 1.0f - (float)i / rings}                // UV
                });
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

void Mesh::CreateCone()
{
    // �⺻ ũ�� ����
    const float radius = 1.0f;
    const float height = 1.0f;
    const int segments = 18;

    vertices.clear();
    indices.clear();

    // �ظ� �߽���
    vertices.push_back({ {0.0f, 0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f} });
    // �ظ� ����
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back({ {x, 0.0f, z}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f} });
    }

    // ����
    unsigned int coneTipIndex = vertices.size();
    vertices.push_back({ {0.0f, height, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 0.5f} });
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * static_cast<float>(M_PI) * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        glm::vec3 normal = glm::normalize(glm::vec3(x, radius / height, z));
        vertices.push_back({ {x, 0.0f, z}, normal, {1.0f, 1.0f, 1.0f}, {(x / radius + 1.0f) * 0.5f, (z / radius + 1.0f) * 0.5f} });
    }

    // �ε��� ����
    // �ظ�
    for (unsigned int i = 1; i <= segments; i++) {
        indices.push_back(0);
        indices.push_back(i + 1);
        indices.push_back(i);
    }
    // ����
    for (unsigned int i = 0; i < segments; i++) {
        indices.push_back(coneTipIndex);
        indices.push_back(coneTipIndex + i + 1);
        indices.push_back(coneTipIndex + i + 2);
    }
    primitivePattern = PrimitivePattern::Triangles;
}