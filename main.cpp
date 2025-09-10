#include <iostream>

#include "SDL3/SDL.h"
#include "glew.h"
#include "SDL3/SDL_opengl.h"

#pragma comment(lib, "opengl32.lib")

#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexArray.hpp"

const char* vertexShaderSource = R"(
        #version 430 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
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

    // 삼각형의 꼭짓점 3개 위치 데이터
    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // 왼쪽 아래
         0.5f, -0.5f, 0.0f, // 오른쪽 아래
         0.0f,  0.5f, 0.0f  // 위쪽 중앙
    };

    //삼각형
    VertexArray va;
    VertexBuffer vb(std::span(vertices), sizeof(vertices));

    va.AddVertexBuffer(std::move(vb), {
        {3, 0, GL_FALSE, 0, GL_FLOAT, 0, 3 * sizeof(float)} 
        });

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
        va.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);


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