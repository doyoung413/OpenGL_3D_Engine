#include "MeshRenderer.hpp"
#include "Engine.hpp"
#include "Object.hpp"
#include "RenderManager.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include <glew.h>
#include <gtc/type_ptr.hpp>

void MeshRenderer::Init()
{
    Engine::GetInstance().GetRenderManager()->Register(this);
}

void MeshRenderer::Update(float /*dt*/)
{
}

void MeshRenderer::End()
{
   Engine::GetInstance().GetRenderManager()->Unregister(this);
}

void MeshRenderer::Render()
{
    if (!mesh || !shader) return;

    if (renderMode == RenderMode::Wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    shader->Bind();
    if (texture)
    {
        shader->SetUniform1i("useTexture", 1);
        texture->Bind(0);
        shader->SetUniform1i("Texture", 0);
    }
    else
    {
        shader->SetUniform1i("useTexture", 0);
    }

    VertexArray* va = mesh->GetVertexArray();
    if (!va) return;
    va->Bind();

    glm::mat4 model = GetOwner()->transform.GetModelMatrix();
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 6.0f, 6.0f), // 카메라 위치
        glm::vec3(0.0f, -6.0f, -6.0f), // 바라보는 지점
        glm::vec3(0.0f, 1.0f, 0.0f)  // 카메라의 위쪽 방향
    );
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    shader->SetUniformMat4f("model", model);
    shader->SetUniformMat4f("view", view);
    shader->SetUniformMat4f("projection", projection);
    shader->SetUniformVec4("Color", color);

    glDrawElements(static_cast<GLenum>(mesh->GetPrimitivePattern()), va->GetIndicesCount(), GL_UNSIGNED_INT, 0);

    if (renderMode == RenderMode::Wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    if (texture) texture->Unbind();
    va->UnBind();
}

void MeshRenderer::CreatePlane()
{
    mesh = std::make_unique<Mesh>();
    mesh->CreatePlane();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateCube()
{
    mesh = std::make_unique<Mesh>();
    mesh->CreateCube();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateSphere()
{
    mesh = std::make_unique<Mesh>();
    mesh->CreateSphere();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateDiamond()
{
    mesh = std::make_unique<Mesh>();
    mesh->CreateDiamond();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateCylinder()
{
    mesh = std::make_unique<Mesh>();
    mesh->CreateCylinder();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateFromData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitivePattern pattern)
{
    mesh = std::make_unique<Mesh>(vertices, indices, pattern);
    mesh->UploadToGPU();
}

void MeshRenderer::SetShader(const std::string& name)
{
    shader = Engine::GetInstance().GetRenderManager()->GetShader(name);
}

void MeshRenderer::SetTexture(const std::string& name)
{
    texture = Engine::GetInstance().GetRenderManager()->GetTexture(name);
}