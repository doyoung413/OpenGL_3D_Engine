#include <iostream>

#include "glew.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_opengl.h"
#pragma comment(lib, "opengl32.lib")

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


int main(int argc, char* argv[]) {
    // SDL 초기화
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL 초기화 실패: " << SDL_GetError() << std::endl;
        return -1;
    }

    // OpenGL 버전 및 프로파일 설정
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4.0);
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
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cout << "GLEW 초기화 실패: " << glewGetErrorString(err) << std::endl;
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    int success;
    char infoLog[512];

    // 버텍스 셰이더 컴파일
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "버텍스 셰이더 컴파일 실패\n" << infoLog << std::endl;
    }
    else {
        std::cout << "버텍스 셰이더 컴파일 성공"<< std::endl;
    }

    // 프래그먼트 셰이더 컴파일
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "프래그먼트 셰이더 컴파일 실패\n" << infoLog << std::endl;
    }
    else {
        std::cout << "프래그먼트 셰이더 컴파일 성공" << std::endl;
    }

    // 셰이더들을 하나의 프로그램으로 링크
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "셰이더 프로그램 링크 실패\n" << infoLog << std::endl;
    }
    else {
        std::cout << "셰이더 프로그램 링크 성공" << std::endl;
    }

    // 셰이더 객체 삭제
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 삼각형의 데이터
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.0f,  0.5f, 0.0f 
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO); // VAO 생성
    glGenBuffers(1, &VBO);      // VBO 생성

    // VAO 활성화
    glBindVertexArray(VAO);

    // VBO 활성화 후 정점 데이터를 GPU 메모리로 복사
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 비활성화
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

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
        glClear(GL_COLOR_BUFFER_BIT);

        // 삼각형
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 더블버퍼링
        SDL_GL_SwapWindow(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    //창 종료
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}