#version 430 core

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
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; 
    
    TexCoord = aTexCoord;
    ourColor = aColor;
}