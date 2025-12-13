#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <atomic>

class ThreadManager {
public:
    ThreadManager() = default;
    ~ThreadManager() = default;

    // 초기화 및 종료 (필요 시 스레드 시작용, 현재는 구조만 유지)
    void Start();
    void Stop();

    // 메인 루프에서 호출되어 이벤트를 처리할 함수
    void ProcessEvents();

private:
    std::atomic<bool> running{ false };
};