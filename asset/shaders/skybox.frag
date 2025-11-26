#version 430 core
out vec4 FragColor;

in vec3 localPos;

uniform samplerCube environmentMap;

void main()
{
    // 큐브맵 샘플링
    vec3 envColor = texture(environmentMap, localPos).rgb;
    
    // HDR Tone Mapping & Gamma Correction (PBR 오브젝트와 톤을 맞춰야 함)
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
  
    FragColor = vec4(envColor, 1.0);
}