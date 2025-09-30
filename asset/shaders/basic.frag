#version 430 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 ourColor;
in vec3 Normal;   // <<< 추가: 버텍스 셰이더로부터 법선 벡터를 받음
in vec3 FragPos;  // <<< 추가: 버텍스 셰이더로부터 위치를 받음

uniform sampler2D Texture; // 'ourTexture'에서 이름 일관성을 위해 변경
uniform bool useTexture;
uniform vec4 color;      // 'Color'에서 소문자로 변경

void main() {
    vec4 baseColor = vec4(1.0);
    if (useTexture)
    {
        baseColor = texture(Texture, TexCoord);
    }
    // 기존의 색상 계산 로직은 그대로 유지
    FragColor = baseColor * vec4(ourColor, 1.0) * color;
}