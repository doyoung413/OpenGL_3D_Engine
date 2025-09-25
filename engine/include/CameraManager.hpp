#pragma once
#include "Camera.hpp"
#include <vector>
#include <memory>

class CameraManager
{
public:
    CameraManager();
    ~CameraManager();

    // 관리하는 모든 카메라를 업데이트 (현재는 메인 카메라만 업데이트)
    void Update(float dt);
    void ClearCameras();

    int CreateCamera();
    // 활성화할 메인 카메라를 설정
    void SetMainCamera(int index);
    // 현재 활성화된 메인 카메라 객체를 반환
    Camera* GetMainCamera();
    Camera* GetCamera(int index);
    const std::vector<std::unique_ptr<Camera>>& GetCameraList() const { return cameraList; }
private:
    std::vector<std::unique_ptr<Camera>> cameraList;
    int mainCameraIndex = -1; // -1은 없음
};