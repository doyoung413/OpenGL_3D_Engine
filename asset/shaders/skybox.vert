#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 localPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    localPos = aPos;
    // 뷰 행렬의 4x4 중 상단 3x3만 사용하여 회전만 적용 (이동 제거)
    mat4 rotView = mat4(mat3(view));
    vec4 clipPos = projection * rotView * vec4(aPos, 1.0);

    // 큐브맵 트릭: z를 w와 같게 하여 깊이 값을 항상 1.0 (최대)로 만듦
    gl_Position = clipPos.xyww;
}