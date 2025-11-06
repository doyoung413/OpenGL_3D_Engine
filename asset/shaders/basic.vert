#version 430 core

const int MAX_BONES = 256;
uniform mat4 finalBonesMatrices[MAX_BONES];

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;  
layout (location = 2) in vec3 aColor;     
layout (location = 3) in vec2 aTexCoord;
layout (location = 4) in ivec4 aBoneIDs;
layout (location = 5) in vec4 aWeights;

out vec3 ourColor;
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
mat4 finalTransform; // 최종 변환 행렬을 담을 변수

    // 뼈 가중치가 있는 애니메이션 정점의 경우
    if (aWeights.x > 0.0)
    {
        mat4 skinningTransform = mat4(0.0f);
        for(int i = 0; i < 4; i++)
        {
            if(aBoneIDs[i] != -1)
            {
                skinningTransform += finalBonesMatrices[aBoneIDs[i]] * aWeights[i];
            }
        }
        // Animator가 계산한 월드+스키닝 행렬을 그대로 사용
        finalTransform = skinningTransform;
    }
    else // 일반 정적 모델의 경우
    {
        // 기존처럼 model 행렬만 사용
        finalTransform = model;
    }

    gl_Position = projection * view * finalTransform * vec4(aPos, 1.0);
    
    vec3 worldPos = vec3(finalTransform * vec4(aPos, 1.0));
    FragPos = worldPos;
    Normal = mat3(transpose(inverse(finalTransform))) * aNormal; 
    
    TexCoord = aTexCoord;
    ourColor = aColor;
}