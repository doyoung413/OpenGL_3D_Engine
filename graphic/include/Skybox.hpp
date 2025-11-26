#pragma once
#include <string>
#include <memory>
#include <vector>
#include "glm.hpp"

class Camera;
class Mesh;

class Skybox
{
public:
    Skybox();
    ~Skybox();

    void Init(const std::string& hdrTexturePath);
    void Render(Camera* camera);
    void BindIBL();

    unsigned int GetIrradianceMap() const { return irradianceMap; }
    unsigned int GetPrefilterMap() const { return prefilterMap; }
    unsigned int GetBrdfLUT() const { return brdfLUTTexture; }

private:
    void PrecomputeIBL(const std::string& hdrPath);
    void RenderQuad(); // BRDF 생성용

    // 텍스처 핸들
    unsigned int envCubemap = 0;
    unsigned int irradianceMap = 0;
    unsigned int prefilterMap = 0;
    unsigned int brdfLUTTexture = 0;

    // 렌더링용 메쉬
    std::shared_ptr<Mesh> skyboxCube;

    // RenderQuad용
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;
};