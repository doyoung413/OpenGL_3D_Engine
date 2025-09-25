#pragma once
#include "Camera.hpp"
#include <vector>
#include <memory>

class CameraManager
{
public:
    CameraManager();
    ~CameraManager();

    // �����ϴ� ��� ī�޶� ������Ʈ (����� ���� ī�޶� ������Ʈ)
    void Update(float dt);
    void ClearCameras();

    int CreateCamera();
    // Ȱ��ȭ�� ���� ī�޶� ����
    void SetMainCamera(int index);
    // ���� Ȱ��ȭ�� ���� ī�޶� ��ü�� ��ȯ
    Camera* GetMainCamera();
    Camera* GetCamera(int index);
    const std::vector<std::unique_ptr<Camera>>& GetCameraList() const { return cameraList; }
private:
    std::vector<std::unique_ptr<Camera>> cameraList;
    int mainCameraIndex = -1; // -1�� ����
};