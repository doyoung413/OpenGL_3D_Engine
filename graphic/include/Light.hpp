#pragma once
#pragma once
#include "Component.hpp"
#include "glm.hpp"

enum class LightType
{
    Directional, // 방향광
    Point        // 점광원
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
    // 방향광의 방향 관련

    void SetOffsetForPointL(const glm::vec3 pos) { offsetForPointL = pos; }
    const glm::vec3& GetOffsetForPointL() const { return offsetForPointL; }

private:
    LightType type = LightType::Directional;

    // 조명 속성
    glm::vec3 color = { 1.0f, 1.0f, 1.0f }; // 빛의 색상 (기본 흰색)
    float ambientIntensity = 0.2f;         // 주변광 강도
    float diffuseIntensity = 0.8f;         // 난반사 강도
    float specularIntensity = 1.0f;        // 정반사 강도
    int shininess = 16;                    // 정반사 하이라이트 크기

    glm::vec3 direction = { 0.f,0.f,0.f }; // 방향광의 방향을 저장할 변수
    glm::vec3 offsetForPointL = { 0.f,0.f,0.f }; // 점광원의 오프셋을 저장할 변수
};