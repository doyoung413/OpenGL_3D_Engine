#version 430 core
out vec4 FragColor;
in vec3 localPos;

uniform sampler2D equirectangularMap; // 입력 (HDR 2D 텍스처)

const vec2 invAtan = vec2(0.1591, 0.3183); // 1 / (2*PI), 1 / PI

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(localPos));
    FragColor = vec4(texture(equirectangularMap, uv).rgb, 1.0);
}