#include "CameraManager.hpp"
#include "Engine.hpp"
#include "InputManager.hpp"

CameraManager::CameraManager()
{
}

CameraManager::~CameraManager() = default;

// ī�޶� ������Ʈ
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
    // unique_ptr�̹Ƿ� clear()�� ȣ��Ǹ� ��� ī�޶� ��ü�� �ڵ����� �Ҹ�˴ϴ�.
    cameraList.clear();
    mainCameraIndex = -1; // ���� ī�޶� �ε��� �ʱ�ȭ
}

// ���ο� ī�޶� ����� �ε����� ��ȯ
int CameraManager::CreateCamera()
{
    cameraList.push_back(std::make_unique<Camera>());
    return static_cast<int>(cameraList.size() - 1);
}

// ���� ī�޶� ����
void CameraManager::SetMainCamera(int index)
{
    if (index >= 0 && index < cameraList.size())
    {
        mainCameraIndex = index;
    }
}

// ���� ���� ī�޶� ��ȯ
Camera* CameraManager::GetMainCamera()
{
    if (mainCameraIndex != -1)
    {
        return cameraList[mainCameraIndex].get();
    }
    return nullptr; // ���� ī�޶� �������� �ʾ��� ���
}

// Ư�� �ε����� ī�޶� ��ȯ
Camera* CameraManager::GetCamera(int index)
{
    if (index >= 0 && index < cameraList.size())
    {
        return cameraList[index].get();
    }
    return nullptr;
}
