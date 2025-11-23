#version 430 core
out vec4 FragColor;
in vec3 localPos;

uniform samplerCube environmentMap; // 입력 (방금 만든 3D 큐브맵)

const float PI = 3.14159265359;

void main()
{		
    vec3 N = normalize(localPos);
    vec3 irradiance = vec3(0.0);   
    
    // 탄젠트 공간 계산
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up        = normalize(cross(N, right));
       
    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // 구면 좌표를 월드 벡터로 변환
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // 탄젠트 공간 -> 월드 공간
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / nrSamples);
    
    FragColor = vec4(irradiance, 1.0);
}