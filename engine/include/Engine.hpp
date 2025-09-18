#pragma once
#include <SDL3/SDL.h>
#include <memory>

class ObjectManager;
class RenderManager;
class InputManager;
class SceneManager;
class Engine
{
public:
    Engine() = default;
    ~Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void Init(int width_, int height_);
    void Run();
    void Shutdown();

    static Engine& GetInstance();
    SDL_Window* GetSDLWindow() { return window; }
    ObjectManager* GetObjectManager() { return objectManager.get(); }
    RenderManager* GetRenderManager() { return renderManager.get(); }
    InputManager* GetInputManager() { return inputManager.get(); }
    SceneManager* GetSceneManager() { return sceneManager.get(); }

    int GetWindowWidth() const { return windowWidth; }
    int GetWindowHeight() const { return windowHeight; }

    void ToggleFullscreen();
private:
    bool isRunning = false;
    int windowWidth = 800;
    int windowHeight = 600;
    bool isFullscreen = false;

    void HandleWindowEvent(const SDL_WindowEvent& windowEvent);

    SDL_Window* window = nullptr;
    SDL_GLContext context;

    std::unique_ptr<ObjectManager> objectManager;
    std::unique_ptr<RenderManager> renderManager;
    std::unique_ptr<InputManager> inputManager;
    std::unique_ptr<SceneManager> sceneManager;
};
