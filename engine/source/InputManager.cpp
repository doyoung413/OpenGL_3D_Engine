#include "Engine.hpp" 
#include "InputManager.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>

void InputManager::Update()
{
    // 현재 키 상태를 이전 키 상태로 복사
    keyStatePrev = keyStates;
    mouseButtonStatePrev = mouseButtonStates;

    // 프레임마다 초기화되어야 할 값들을 리셋
    mouseWheelMotion = { 0.0f, 0.0f };
    relativeMouseMotion = { 0.0f, 0.0f };
}

void InputManager::PollEvent(SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
    ImGuiIO& io = ImGui::GetIO();
    if ((/*io.WantCaptureMouse ||*/ io.WantCaptureKeyboard) && !GetRelativeMouseMode())
    {
        return;
    }

    switch (event.type)
    {
    case SDL_EVENT_KEY_DOWN:
        KeyDown(static_cast<KEYBOARDKEYS>(event.key.key));
        break;
    case SDL_EVENT_KEY_UP:
        KeyUp(static_cast<KEYBOARDKEYS>(event.key.key));
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        MouseButtonDown(static_cast<MOUSEBUTTON>(event.button.button));
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        MouseButtonUp(static_cast<MOUSEBUTTON>(event.button.button));
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        MouseWheel(event);
        break;
    case SDL_EVENT_MOUSE_MOTION:
        relativeMouseMotion.x = static_cast<float>(event.motion.xrel);
        relativeMouseMotion.y = static_cast<float>(event.motion.yrel);
        break;
    }
}

// 키가 계속 눌리고 있는지 확인
bool InputManager::IsKeyPressed(KEYBOARDKEYS keycode)
{
    return keyStates[keycode];
}

// 키가 이번 프레임에 처음 눌렸는지 확인
bool InputManager::IsKeyPressOnce(KEYBOARDKEYS keycode)
{
    return keyStates[keycode] && !keyStatePrev[keycode];
}

// 키가 이번 프레임에 떼어졌는지 확인
bool InputManager::IsKeyReleasedOnce(KEYBOARDKEYS keycode)
{
    return !keyStates[keycode] && keyStatePrev[keycode];
}

// 마우스 버튼이 계속 눌리고 있는지 확인
bool InputManager::IsMouseButtonPressed(MOUSEBUTTON button)
{
    return mouseButtonStates[button];
}

// 마우스 버튼이 이번 프레임에 처음 눌렸는지 확인
bool InputManager::IsMouseButtonPressOnce(MOUSEBUTTON button)
{
    return mouseButtonStates[button] && !mouseButtonStatePrev[button];
}

// 마우스 버튼이 이번 프레임에 떼어졌는지 확인
bool InputManager::IsMouseButtonReleasedOnce(MOUSEBUTTON button)
{
    return !mouseButtonStates[button] && mouseButtonStatePrev[button];
}

// 현재 마우스 위치 반환
glm::vec2 InputManager::GetMousePosition()
{
    float mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    return { mouseX, mouseY };
}

glm::vec2 InputManager::GetMouseWheelMotion()
{
    return mouseWheelMotion;
}

glm::vec2 InputManager::GetRelativeMouseMotion()
{
    return relativeMouseMotion;
}

void InputManager::MouseWheel(const SDL_Event& event)
{
    mouseWheelMotion.x = event.wheel.x;
    mouseWheelMotion.y = event.wheel.y;
}


void InputManager::SetRelativeMouseMode(bool state)
{
    SDL_Window* window = Engine::GetInstance().GetSDLWindow();
    SDL_SetWindowRelativeMouseMode(window, state);
}

bool InputManager::GetRelativeMouseMode()
{
    SDL_Window* window = Engine::GetInstance().GetSDLWindow();
    return SDL_GetWindowRelativeMouseMode(window);
}