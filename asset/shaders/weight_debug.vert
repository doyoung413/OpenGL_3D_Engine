#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 5) in vec4 aWeights; 

out vec4 vertWeights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertWeights = aWeights;
}