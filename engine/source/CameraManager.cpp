#include "CameraManager.hpp"
#include "Engine.hpp"
#include "InputManager.hpp"

CameraManager::CameraManager()
{
}

CameraManager::~CameraManager() = default;

// 카메라 업데이트
void CameraManager::Update(float /*dt*/)
{
    for (const auto& cameraPtr : cameraList)
    {
        if (cameraPtr != nullptr)
        {
            cameraPtr->Update();
        }
    }
}

void CameraManager::ClearCameras()
{
    // unique_ptr이므로 clear()가 호출되면 모든 카메라 객체는 자동으로 소멸됩니다.
    cameraList.clear();
    mainCameraIndex = -1; // 메인 카메라 인덱스 초기화
}

// 새로운 카메라를 만들고 인덱스를 반환
int CameraManager::CreateCamera()
{
    cameraList.push_back(std::make_unique<Camera>());
    return static_cast<int>(cameraList.size() - 1);
}

// 메인 카메라 설정
void CameraManager::SetMainCamera(int index)
{
    if (index >= 0 && index < cameraList.size())
    {
        mainCameraIndex = index;
    }
}

// 현재 메인 카메라 반환
Camera* CameraManager::GetMainCamera()
{
    if (mainCameraIndex != -1)
    {
        return cameraList[mainCameraIndex].get();
    }
    return nullptr; // 메인 카메라가 설정되지 않았을 경우
}

// 특정 인덱스의 카메라 반환
Camera* CameraManager::GetCamera(int index)
{
    if (index >= 0 && index < cameraList.size())
    {
        return cameraList[index].get();
    }
    return nullptr;
}
