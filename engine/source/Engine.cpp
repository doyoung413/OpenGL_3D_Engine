#include "Engine.hpp"
#include "ObjectManager.hpp"
#include "RenderManager.hpp"
#include "InputManager.hpp"
#include "SceneManager.hpp"
#include "CameraManager.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <glew.h>
#include <iostream>

Engine& Engine::GetInstance()
{
    static Engine instance;
    return instance;
}

void Engine::Init(int width_, int height_)
{
    objectManager = std::make_unique<ObjectManager>();
    renderManager = std::make_unique<RenderManager>();
    inputManager = std::make_unique<InputManager>();
    sceneManager = std::make_unique<SceneManager>();
    cameraManager = std::make_unique<CameraManager>();

    if (!SDL_Init(SDL_INIT_VIDEO)) 
    { 
        std::cout << "SDL 초기화 실패: " << SDL_GetError() << std::endl; 
        return; 
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 

    windowWidth = width_;
    windowHeight = height_;

    window = SDL_CreateWindow("OpenGL", windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) 
    {
        std::cout << "윈도우 생성 실패: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return; 
    }
    context = SDL_GL_CreateContext(window);
    if (!context)
    {
        std::cout << "OpenGL 컨텍스트 생성 실패: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW 초기화 실패: " << std::endl;
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    glEnable(GL_DEPTH_TEST);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImFontConfig font_config;
    font_config.SizePixels = 18.0f;
    io.Fonts->AddFontDefault(&font_config);
    ImGui::GetStyle().ScaleAllSizes(2.f); // UI 요소들의 크기도 함께 조절

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 키보드 컨트롤 활성화

    ImGui::StyleColorsDark(); // 기본 다크 테마 설정

    // SDL3와 OpenGL3 백엔드 초기화
    ImGui_ImplSDL3_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init("#version 430"); // GLSL 버전 문자열

    std::cout << "Engine Initialized Successfully!" << std::endl;
    isRunning = true;
}

void Engine::Run()
{
    while (isRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                isRunning = false;
                break;
            case SDL_EVENT_WINDOW_MOVED:
            case SDL_EVENT_WINDOW_RESIZED:
            case SDL_EVENT_WINDOW_MINIMIZED:
            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_RESTORED:
                HandleWindowEvent(event.window);
                break;
            }

            inputManager->PollEvent(event);
        }

        // 로직 업데이트 및 렌더링
        sceneManager->Update(1.f / 60.f);
        inputManager->Update();
    }
}

void Engine::Shutdown()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    objectManager->DestroyAllObjects();
    renderManager->ResetAllResources();
    cameraManager->ClearCameras();

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    std::cout << "Engine Shutdown" << std::endl;
}

void Engine::ToggleFullscreen()
{
    isFullscreen = !isFullscreen;
    SDL_SetWindowFullscreen(window, isFullscreen);
    std::cout << "Fullscreen mode " << (isFullscreen ? "ON" : "OFF") << std::endl;
}

void Engine::HandleWindowEvent(const SDL_WindowEvent& windowEvent)
{
    switch (windowEvent.type)
    {
    case SDL_EVENT_WINDOW_MOVED:
        std::cout << "Dragged to (" << windowEvent.data1 << ", " << windowEvent.data2 << ")" << std::endl;
        break;

    case SDL_EVENT_WINDOW_RESIZED:
    {
        windowWidth = windowEvent.data1;
        windowHeight = windowEvent.data2;
        std::cout << "[Window] Resized to " << windowWidth << "x" << windowHeight << std::endl;
        break;
        break;
    }

    case SDL_EVENT_WINDOW_MINIMIZED:
        std::cout << "Minimized" << std::endl;
        break;

    case SDL_EVENT_WINDOW_MAXIMIZED:
        std::cout << "Maximized" << std::endl;
        break;

    case SDL_EVENT_WINDOW_RESTORED:
        std::cout << "Restored" << std::endl;
        break;
    }
}
