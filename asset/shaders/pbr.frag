#version 430 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 ourColor;
in vec3 Normal;  
in vec3 FragPos; // 월드 공간 위치

// Light (basic.frag와 동일)
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos; // 카메라(뷰)의 월드 공간 위치
uniform bool isPointLight;

// Material (MeshRenderer에서 전달받음)
uniform sampler2D Texture; 
uniform bool useTexture;
uniform vec4 color;       // albedo
uniform float metallic;
uniform float roughness;

const float PI = 3.14159265359;


// D: Normal Distribution Function (GGX)
// 미세표면의 노멀이 'h' 벡터를 향해 얼마나 정렬되었는지
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return num / denom;
}

// G: Geometry Function (Schlick-GGX)
// 미세표면의 그림자(Shadowing/Masking) 효과
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// F: Fresnel Equation (Schlick Approximation)
// 표면 법선과 시선 각도에 따라 반사되는 빛의 비율
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 재질 속성 준비
    vec4 albedoTex = vec4(ourColor, 1.0) * color;
    if (useTexture) {
        albedoTex = texture(Texture, TexCoord);
    }
    vec3 albedo = albedoTex.rgb;
    
    // F0 (0도에서 반사율): 비금속(0.04)과 금속(albedo) 사이를 보간
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

    // 입력 벡터 준비
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos); // View
    
    vec3 L; // Light
    if (isPointLight) {
        L = normalize(lightPos - FragPos);
    } else {
        L = normalize(-lightPos); // 방향광
    }
    vec3 H = normalize(V + L); // Halfway

    // 조명 계산 (Radiance)
    vec3 Lo = vec3(0.0); 

    // Cook-Torrance BRDF (Specular)
    float NdotL = max(dot(N, L), 0.0);
    if(NdotL > 0.0)
    {
        // D, G, F 계산
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        // Specular BRDF 분모
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001; // 0 방지
        vec3 specular = D * G * F / denominator;
        
        // kS (Specular 비율) = Fresnel
        // kD (Diffuse 비율) = (1 - kS) * (1 - metallic)
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= (1.0 - metallic); // 금속은 Diffuse가 0
        
        // 최종 합산 (lightColor는 Phong의 'diffuseStrength'와 같음)
        // L·N은 항상 곱해짐
        Lo += (kD * albedo / PI + specular) * lightColor * NdotL;
    }

    // 수정] Ambient (차후 12주차에 IBL로 대체될 임시항목)
    // Phong의 ambientStrength를 재활용
    vec3 ambient = 0.03 * albedo; // 매우 약한 기본 주변광
    
    vec3 colorResult = ambient + Lo;

    // HDR 톤 매핑 (임시) - 너무 밝은 PBR 값을 LDR로 압축
    colorResult = colorResult / (colorResult + vec3(1.0));
    colorResult = pow(colorResult, vec3(1.0/2.2)); // 감마 보정

    FragColor = vec4(colorResult, albedoTex.a);
}