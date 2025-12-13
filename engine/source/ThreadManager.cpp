#include "ThreadManager.hpp"
#include "Engine.hpp"
#include "InputManager.hpp"
#include "RenderManager.hpp" // 리사이징 처리를 위해 필요하다면 추가
#include "SceneManager.hpp"  // 종료 처리를 위해 필요

#include "imgui.h"
#include "imgui_impl_sdl3.h"

#include <iostream>

void ThreadManager::Start()
{
    running = true;
}

void ThreadManager::Stop()
{
    running = false;
    // 스레드 join 로직이 있다면 여기서 처리
}

void ThreadManager::ProcessEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        // ImGui 이벤트 처리
        ImGui_ImplSDL3_ProcessEvent(&event);

        // 엔진 입력 매니저로 전달
        Engine::GetInstance().GetInputManager()->PollEvent(event);

        // 윈도우 및 시스템 이벤트 처리
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            Engine::GetInstance().Quit();
            break;

        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
            Engine::GetInstance().HandleWindowEvent(event.window);
            break;
        }
    }
}