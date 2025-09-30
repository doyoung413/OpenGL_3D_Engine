#version 430 core

// C++의 Vertex 구조체 레이아웃과 순서를 일치시킵니다.
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;     // <<< 추가: 법선 벡터 입력
layout (location = 2) in vec3 aColor;      // <<< location 변경 (1 -> 2)
layout (location = 3) in vec2 aTexCoord;   // <<< location 변경 (2 -> 3)

// 프래그먼트 셰이더로 전달할 출력 변수들
out vec3 ourColor;
out vec2 TexCoord;
out vec3 Normal;       // <<< 추가: 법선 벡터 전달
out vec3 FragPos;      // <<< 추가: 월드 좌표 위치 전달

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    
    // 월드 공간에서의 정점 위치와 법선을 프래그먼트 셰이더로 전달 (6주차 조명에서 사용)
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; // 법선 벡터 변환
    
    TexCoord = aTexCoord;
    ourColor = aColor;
}