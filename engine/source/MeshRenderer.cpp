#include "MeshRenderer.hpp"

#include "Model.hpp"
#include "Light.hpp"

#include "Engine.hpp"
#include "Object.hpp"
#include "RenderManager.hpp"
#include "CameraManager.hpp"
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

void MeshRenderer::Render(Camera* camera, Light* light)
{// �������� ������(model �Ǵ� mesh)�� ���̴�, ī�޶� ������ �Լ� ����
    if ((!model && !mesh) || !shader || !camera)
    {
        return;
    }

    if (renderMode == RenderMode::Wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
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

    // ���� ������(���, ƾƮ ���� ��) ����
    glm::mat4 modelMat = GetOwner()->transform.GetModelMatrix();
    glm::mat4 viewMat = camera->GetViewMatrix();
    glm::mat4 projectionMat = camera->GetProjectionMatrix();
    shader->SetUniformMat4f("model", modelMat);
    shader->SetUniformMat4f("view", viewMat);
    shader->SetUniformMat4f("projection", projectionMat);
    shader->SetUniformVec4("color", color);

    // ���� ������ ���� (���� light ���ڸ� ���� ���)
    if (light)
    {
        Object* lightObject = light->GetOwner();
        bool isPoint = (light->GetType() == LightType::Point);

        if (isPoint)
        {
            shader->SetUniformVec3("lightPos", lightObject->transform.GetPosition() + light->GetOffsetForPointL());
        }
        else
        {
            shader->SetUniformVec3("lightPos", light->GetDirection());
        }
        shader->SetUniform1i("isPointLight", isPoint);

        shader->SetUniformVec3("lightColor", light->GetColor());
        shader->SetUniformVec3("viewPos", camera->GetCameraPosition());
        shader->SetUniform1f("ambientStrength", light->GetAmbientIntensity());
        shader->SetUniform1f("diffuseStrength", light->GetDiffuseIntensity());
        shader->SetUniform1f("specularStrength", light->GetSpecularIntensity());
        shader->SetUniform1i("shininess", light->GetShininess());
    }
    else // ���� ������ �⺻������ ����
    {
        shader->SetUniformVec3("lightPos", { 0,0,0 });
        shader->SetUniformVec3("lightColor", { 0,0,0 }); 
    }

    if (model) // �� �����Ͱ� �ִٸ�
    {
        // ���� ���� ��� �޽��� ��ȸ�ϸ� ������
        for (const auto& mesh_in_model : model->GetMeshes())
        {
            VertexArray* va = mesh_in_model->GetVertexArray();
            if (!va) continue;
            va->Bind();
            glDrawElements(static_cast<GLenum>(mesh_in_model->GetPrimitivePattern()), mesh_in_model->GetIndicesCount(), GL_UNSIGNED_INT, 0);
            va->UnBind();
        }
    }
    else if (mesh) // ���� �޽� �����Ͱ� �ִٸ�
    {
        // ���� �޽��� ������
        VertexArray* va = mesh->GetVertexArray();
        if (va)
        {
            va->Bind();
            glDrawElements(static_cast<GLenum>(mesh->GetPrimitivePattern()), mesh->GetIndicesCount(), GL_UNSIGNED_INT, 0);
            va->UnBind();
        }
    }

    if (renderMode == RenderMode::Wireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    if (texture)
    {
        texture->Unbind();
    }
}

void MeshRenderer::CreatePlane()
{
    model = nullptr;
    mesh = std::make_unique<Mesh>();
    mesh->CreatePlane();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateCube()
{
    model = nullptr;
    mesh = std::make_unique<Mesh>();
    mesh->CreateCube();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateSphere()
{
    model = nullptr;
    mesh = std::make_unique<Mesh>();
    mesh->CreateSphere();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateDiamond()
{
    model = nullptr;
    mesh = std::make_unique<Mesh>();
    mesh->CreateDiamond();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateCylinder()
{
    model = nullptr;
    mesh = std::make_unique<Mesh>();
    mesh->CreateCylinder();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateCapsule()
{
    model = nullptr;
    mesh = std::make_unique<Mesh>();
    mesh->CreateCapsule();
    mesh->UploadToGPU();
}

void MeshRenderer::CreateFromData(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, PrimitivePattern pattern)
{
    model = nullptr;
    mesh = std::make_unique<Mesh>(vertices, indices, pattern);
    mesh->UploadToGPU();
}

void MeshRenderer::LoadModel(const std::string& path)
{
    mesh = nullptr;
    model = std::make_shared<Model>(path);
    for (const auto& mesh_in_model : model->GetMeshes()) {
        mesh_in_model->UploadToGPU();
    }
}

void MeshRenderer::SetShader(const std::string& name)
{
    shader = Engine::GetInstance().GetRenderManager()->GetShader(name);
}

void MeshRenderer::SetTexture(const std::string& name)
{
    texture = Engine::GetInstance().GetRenderManager()->GetTexture(name);
}