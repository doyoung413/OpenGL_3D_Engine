#version 430 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 ourColor;
in vec3 Normal;  
in vec3 FragPos;

//Light
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float ambientStrength;
uniform float diffuseStrength;
uniform float specularStrength;
uniform int shininess;
uniform bool isPointLight;
//Light

uniform sampler2D Texture; 
uniform bool useTexture;
uniform vec4 color;

void main() {
    vec4 baseColor = vec4(ourColor, 1.0) * color;
    if (useTexture)
    {
        baseColor = texture(Texture, TexCoord);
    }

    vec3 norm = normalize(Normal);
    vec3 lightDir;

    if (isPointLight)
    {
        // 점광원: 프래그먼트 위치를 기준으로 빛의 방향 계산
        lightDir = normalize(lightPos - FragPos);
    }
    else
    {
        // 방향광: 모든 프래그먼트에서 빛의 방향이 동일 (lightPos에 방향 벡터가 들어옴)
        lightDir = normalize(-lightPos);
    }

    // 주변광 (Ambient)
    vec3 ambient = ambientStrength * lightColor;

    // 난반사 (Diffuse)
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * diff * lightColor;

    // 정반사 (Specular)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;
    
    // 최종 색상 조합
    // 주변광과 난반사는 물체 고유의 색에 영향을 주고, 정반사는 하이라이트로 더해짐
    vec3 result = (ambient + diffuse) * baseColor.rgb + specular;
    FragColor = vec4(result, baseColor.a);
}