#version 430 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 ourColor;
in vec3 Normal;  
in vec3 FragPos;
// 월드 공간 위치

// Light (기존 직접 조명용)
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
// 카메라(뷰)의 월드 공간 위치
uniform bool isPointLight;

// Material
layout(binding = 0) uniform sampler2D Texture; 
uniform bool useTexture;
uniform vec4 color;
uniform float metallic;
uniform float roughness;
uniform float exposure;

// 간접 조명 맵
layout(binding = 1) uniform samplerCube irradianceMap; // 확산광용 (Diffuse)
layout(binding = 2) uniform samplerCube prefilterMap; // 반사광용 (Specular)

// 반사광용 (Specular)
layout(binding = 3) uniform sampler2D brdfLUT;         // BRDF 조화 테이블

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
    return num / max(denom, 0.0000001); // 0 방지
}

// G: Geometry Function (Schlick-GGX)
// 미세표면의 그림자(Shadowing/Masking) 효과
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; // 직접광용 k값
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

// [IBL 추가] 거칠기를 고려한 Fresnel (간접광용)
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 재질 속성 준비
    vec4 albedoTex = vec4(ourColor, 1.0) * color;
    if (useTexture) {
        albedoTex = texture(Texture, TexCoord);
    }
    // sRGB -> Linear 변환 (PBR 필수)
    vec3 albedo = pow(albedoTex.rgb, vec3(2.2));
    
    // F0 (0도에서 반사율): 비금속(0.04)과 금속(albedo) 사이를 보간
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // 입력 벡터 준비
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos); // View
    vec3 R = reflect(-V, N); // 반사 벡터 (IBL Specular용)
    
    // 1. 직접 조명 계산
    vec3 Lo = vec3(0.0);
    
    vec3 L;
    // Light
    if (isPointLight) {
        L = normalize(lightPos - FragPos);
    } else {
        L = normalize(-lightPos); // 방향광
    }
    vec3 H = normalize(V + L); // Halfway
    float NdotL = max(dot(N, L), 0.0);

    if(NdotL > 0.0) // 빛을 받는 면만 계산
    {
        // D, G, F 계산
        float D = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
        
        // Specular BRDF 분모
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
        vec3 specular = D * G * F / denominator;
        
        // kS (Specular 비율) = Fresnel
        vec3 kS = F;
        // kD (Diffuse 비율) = (1 - kS) * (1 - metallic)
        vec3 kD = vec3(1.0) - kS;
        kD *= (1.0 - metallic); // 금속은 Diffuse가 0
        
        vec3 radiance = lightColor;
        
        if (isPointLight)
        {
            float distance = length(lightPos - FragPos);
            // 거리가 멀어질수록 빛이 약해짐 (거리의 제곱 반비례)
            float attenuation = 1.0 / (distance * distance); 
            radiance *= attenuation;
        }

        // 최종 합산
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    // 2. IBL (환경 조명) 계산
    // 2-1. 확산광 (Diffuse)
    vec3 F_IBL = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS_IBL = F_IBL;
    vec3 kD_IBL = 1.0 - kS_IBL;
    kD_IBL *= (1.0 - metallic); // 금속은 Diffuse가 0
    
    // Irradiance Map 샘플링 (간접 확산광)
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse    = irradiance * albedo;

    // 2-2. 반사광 (Specular)
    const float MAX_REFLECTION_LOD = 4.0;
    // Prefilter Map 샘플링 (거칠기에 따라 LOD 레벨 선택)
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    
    // BRDF LUT 샘플링 (x: 스케일, y: 바이어스)
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    
    // 3. 최종 IBL Specular 공식 적용
    vec3 specular = prefilteredColor * (F0 * brdf.x + brdf.y);

    // 간접광 = Diffuse + Specular
    vec3 ambient = (kD_IBL * diffuse + specular); // AO는 일단 1.0 처리
    vec3 colorResult = ambient + Lo;

    // 노출(Exposure) 적용
    colorResult *= exposure; 

    // HDR 톤 매핑 (Reinhard)
    colorResult = colorResult / (colorResult + vec3(1.0));
    
    // 감마 보정
    colorResult = pow(colorResult, vec3(1.0/2.2));
    FragColor = vec4(colorResult, albedoTex.a);
}