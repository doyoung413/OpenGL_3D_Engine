// asset/shaders/pbr.vert (basic.vert와 동일)
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
out vec3 FragPos; // 월드 공간에서의 위치

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    mat4 finalTransform;

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
        finalTransform = skinningTransform;
    }
    else
    {
        finalTransform = model;
    }

    gl_Position = projection * view * finalTransform * vec4(aPos, 1.0);
    
    // PBR은 월드 공간 기준 계산이 필수
    FragPos = vec3(finalTransform * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(finalTransform))) * aNormal; 
    
    TexCoord = aTexCoord;
    ourColor = aColor;
}