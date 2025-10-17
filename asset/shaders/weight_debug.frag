#version 430 core
out vec4 FragColor;

in vec4 vertWeights;

void main()
{
    // 가장 영향력이 큰 뼈의 가중치를 가져옴
    float weight = vertWeights.x;

    // 가중치 값(0.0 ~ 1.0)을 색상(파랑 -> 빨강)으로 매핑
    vec3 color = vec3(weight, 0.0, 1.0 - weight);

    FragColor = vec4(color, 1.0);
}