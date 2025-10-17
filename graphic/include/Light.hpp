#pragma once
#pragma once
#include "Component.hpp"
#include "glm.hpp"

enum class LightType
{
    Directional, // ���Ɽ
    Point        // ������
};

class Light : public Component
{
public:
    Light() : Component(ComponentTypes::LIGHT) {}
    
    void Init() override;
    void End() override;
    void Update(float /*dt*/) override {};

    void SetColor(const glm::vec3& color_) { color = color_; }
    const glm::vec3& GetColor() const { return color; }

    void SetAmbientIntensity(float intensity) { ambientIntensity = intensity; }
    float GetAmbientIntensity() const { return ambientIntensity; }

    void SetDiffuseIntensity(float intensity) { diffuseIntensity = intensity; }
    float GetDiffuseIntensity() const { return diffuseIntensity; }

    void SetSpecularIntensity(float intensity) { specularIntensity = intensity; }
    float GetSpecularIntensity() const { return specularIntensity; }

    void SetShininess(int shininess_) { shininess = shininess_; }
    int GetShininess() const { return shininess; }

    void SetType(LightType type_) { type = type_; }
    LightType GetType() const { return type; }

    void SetDirection(const glm::vec3& direction_) { direction = glm::normalize(direction_); }
    const glm::vec3& GetDirection() const { return direction; }
    // ���Ɽ�� ���� ����

    void SetOffsetForPointL(const glm::vec3 pos) { offsetForPointL = pos; }
    const glm::vec3& GetOffsetForPointL() const { return offsetForPointL; }

private:
    LightType type = LightType::Directional;

    // ���� �Ӽ�
    glm::vec3 color = { 1.0f, 1.0f, 1.0f }; // ���� ���� (�⺻ ���)
    float ambientIntensity = 0.2f;         // �ֺ��� ����
    float diffuseIntensity = 0.8f;         // ���ݻ� ����
    float specularIntensity = 1.0f;        // ���ݻ� ����
    int shininess = 16;                    // ���ݻ� ���̶���Ʈ ũ��

    glm::vec3 direction = { 0.f,0.f,0.f }; // ���Ɽ�� ������ ������ ����
    glm::vec3 offsetForPointL = { 0.f,0.f,0.f }; // �������� �������� ������ ����
};