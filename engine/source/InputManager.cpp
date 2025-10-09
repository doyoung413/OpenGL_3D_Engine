#include "Engine.hpp" 
#include "InputManager.hpp"

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include <SDL3/SDL.h>

void InputManager::Update()
{
    // ���� Ű ���¸� ���� Ű ���·� ����
    keyStatePrev = keyStates;
    mouseButtonStatePrev = mouseButtonStates;

    // �����Ӹ��� �ʱ�ȭ�Ǿ�� �� ������ ����
    mouseWheelMotion = { 0.0f, 0.0f };
    relativeMouseMotion = { 0.0f, 0.0f };
}

void InputManager::PollEvent(SDL_Event& event)
{
    ImGui_ImplSDL3_ProcessEvent(&event);
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
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

// Ű�� ��� ������ �ִ��� Ȯ��
bool InputManager::IsKeyPressed(KEYBOARDKEYS keycode)
{
    return keyStates[keycode];
}

// Ű�� �̹� �����ӿ� ó�� ���ȴ��� Ȯ��
bool InputManager::IsKeyPressOnce(KEYBOARDKEYS keycode)
{
    return keyStates[keycode] && !keyStatePrev[keycode];
}

// Ű�� �̹� �����ӿ� ���������� Ȯ��
bool InputManager::IsKeyReleasedOnce(KEYBOARDKEYS keycode)
{
    return !keyStates[keycode] && keyStatePrev[keycode];
}

// ���콺 ��ư�� ��� ������ �ִ��� Ȯ��
bool InputManager::IsMouseButtonPressed(MOUSEBUTTON button)
{
    return mouseButtonStates[button];
}

// ���콺 ��ư�� �̹� �����ӿ� ó�� ���ȴ��� Ȯ��
bool InputManager::IsMouseButtonPressOnce(MOUSEBUTTON button)
{
    return mouseButtonStates[button] && !mouseButtonStatePrev[button];
}

// ���콺 ��ư�� �̹� �����ӿ� ���������� Ȯ��
bool InputManager::IsMouseButtonReleasedOnce(MOUSEBUTTON button)
{
    return !mouseButtonStates[button] && mouseButtonStatePrev[button];
}

// ���� ���콺 ��ġ ��ȯ
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