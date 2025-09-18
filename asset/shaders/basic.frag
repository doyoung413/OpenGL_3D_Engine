#version 430 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 ourColor;

uniform sampler2D Texture;
uniform bool useTexture;
uniform vec4 Color; 

void main() {
    vec4 baseColor = vec4(1.0);
    if (useTexture)
    {
        baseColor = texture(Texture, TexCoord);
    }
    FragColor = baseColor * vec4(ourColor, 1.0) * Color;
}