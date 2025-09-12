#include <iostream>

#include "SDL3/SDL.h"
#include "glew.h"
#include "SDL3/SDL_opengl.h"

#pragma comment(lib, "opengl32.lib")

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"
#include "Texture.hpp"


const char* vertexShaderSource = R"(
        #version 430 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;

        out vec2 TexCoord;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";

const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        in vec2 TexCoord;
        uniform sampler2D ourTexture;

        void main() {
            //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
            FragColor = texture(ourTexture, TexCoord);
        }
    )";


// 셰이더 컴파일 헬퍼 함수
unsigned int CompileShaders() {
    int success;
    char infoLog[512];
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); std::cerr << "VS-ERROR: " << infoLog << std::endl; return 0; }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog); std::cerr << "FS-ERROR: " << infoLog << std::endl; return 0; }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) { glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog); std::cerr << "LINK-ERROR: " << infoLog << std::endl; return 0; }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

int main(int argc, char* argv[]) {
    // SDL 초기화
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL 초기화 실패: " << SDL_GetError() << std::endl;
        return -1;
    }

    // OpenGL 버전 및 프로파일 설정
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // 윈도우 생성
    SDL_Window* window = SDL_CreateWindow(
        "OpenGL",
        800,                              
        600,                              
        SDL_WINDOW_OPENGL                 
    );
    if (!window) {
        std::cout << "윈도우 생성 실패: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // OpenGL 컨텍스트 생성
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        std::cout << "OpenGL 컨텍스트 생성 실패: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // GLEW 초기화
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW 초기화 실패: " << std::endl;
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 깊이 테스트 활성화 (3D 객체가 제대로 보이게 함)
    glEnable(GL_DEPTH_TEST);
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    unsigned int shaderProgram;
    shaderProgram = CompileShaders();

    // 큐브 데이터
    float vertices[] = {
        // 위치               // 텍스처 좌표
        // 뒷면
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        // 앞면
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        // 왼쪽 면
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        // 오른쪽 면
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         // 아랫면
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
          0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         // 윗면
         -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
          0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  0.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,   2, 3, 0,
        4, 5, 6,   6, 7, 4,
        8, 9, 10,  10, 11, 8,
        12, 13, 14, 14, 15, 12,
        16, 17, 18, 18, 19, 16,
        20, 21, 22, 22, 23, 20
    };

    //큐브
    VertexArray va;
    VertexBuffer vb{ std::span<float>(vertices), sizeof(vertices) };
    IndexBuffer ib{ std::span<const unsigned int>(indices) };

    va.AddVertexBuffer(std::move(vb), {
        // 속성 0: 위치 
        {3, 0, GL_FALSE, 0, GL_FLOAT, 0, 5 * sizeof(float)},
        // 속성 1: 텍스처
        {2, 1, GL_FALSE, 0, GL_FLOAT, 3 * sizeof(float), 5 * sizeof(float)}
        });
    va.AddIndexBuffer(std::move(ib));

    Texture texture("asset/wall.jpg");

    bool isRunning = true;
    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }

        // 배경
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 삼각형
        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "test"), 0);

        //큐브를 계속 회전
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(glm::mat4(1.0f), (float)SDL_GetTicks() / 1000.0f, glm::vec3(0.5f, 1.0f, 0.0f));
        
        glm::mat4 view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f), // 카메라 위치
            glm::vec3(0.0f, 0.0f, 0.0f), // 바라보는 지점
            glm::vec3(0.0f, 1.0f, 0.0f)  // 카메라의 위쪽 방향
        );

        //45도 시야각으로 원근감을 적용
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // 행렬 데이터를 셰이더로 전송
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        texture.Bind(0);
        va.Bind();
        glDrawElements(GL_TRIANGLES, va.GetIndicesCount(), GL_UNSIGNED_INT, 0);

        // 더블버퍼링
        SDL_GL_SwapWindow(window);
    }
    glDeleteProgram(shaderProgram);

    //창 종료
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}