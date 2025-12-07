#include "CameraManager.hpp"
#include "Engine.hpp"
#include "InputManager.hpp"

#include "imgui.h"
#include <string>
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

void CameraManager::DeleteCamera(int index)
{
    // 범위 확인 및 메인 카메라 삭제 방지
    if (index < 0 || index >= cameraList.size()) return;
    if (index == mainCameraIndex) return; // 메인 카메라는 삭제 불가

    // 벡터에서 제거
    cameraList.erase(cameraList.begin() + index);

    // 삭제된 카메라보다 뒤에 있던 메인 카메라 인덱스를 하나 당김
    if (mainCameraIndex > index)
    {
        mainCameraIndex--;
    }

    // 삭제된 카메라가 현재 선택된 카메라였다면, 이전 카메라(혹은 0번)를 선택
    if (selectedCameraIndex == index)
    {
        selectedCameraIndex = std::max(0, index - 1);
    }
    else if (selectedCameraIndex > index)
    {
        selectedCameraIndex--;
    }
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

void CameraManager::CameraControllerForImGui()
{
    bool shouldDelete = false;
    ImGui::Begin("Camera Manager");

    if (cameraList.empty())
    {
        ImGui::Text("No cameras available.");
        ImGui::End();
        return;
    }

    std::string currentCameraName = "Camera " + std::to_string(selectedCameraIndex);
    if (mainCameraIndex == selectedCameraIndex) currentCameraName += " (Main)";

    if (ImGui::BeginCombo("Select Camera", currentCameraName.c_str()))
    {
        for (int i = 0; i < cameraList.size(); ++i)
        {
            bool isSelected = (selectedCameraIndex == i);
            std::string name = "Camera " + std::to_string(i);
            if (mainCameraIndex == i) name += " (Main)";

            if (ImGui::Selectable(name.c_str(), isSelected))
            {
                selectedCameraIndex = i;
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("Set as Main"))
    {
        SetMainCamera(selectedCameraIndex);
    }

    if (ImGui::Button("Create New Camera"))
    {
        CreateCamera();
    }

    ImGui::Separator();

    Camera* cam = GetCamera(selectedCameraIndex);
    if (cam)
    {
        std::string title = "Camera " + std::to_string(selectedCameraIndex) + " Settings";
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), title.c_str());
        ImGui::Separator();

        // 메인 카메라 설정 버튼
        if (selectedCameraIndex != mainCameraIndex)
        {
            if (ImGui::Button("Set as Main Camera"))
            {
                SetMainCamera(selectedCameraIndex);
            }
            ImGui::SameLine();

            // 메인이 아닐 때만 활성화 (빨간색 버튼)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("Delete"))
            {
                // 여기서 바로 삭제하거나 return 하지 않고, 플래그만 켭니다.
                shouldDelete = true;
            }
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[Current Main Camera]");
            ImGui::Text("(Cannot be deleted)");
        }
        ImGui::Separator();

        // 위치
        glm::vec3 pos = cam->GetCameraPosition();
        float posArr[3] = { pos.x, pos.y, pos.z };
        if (ImGui::DragFloat3("Position", posArr, 0.1f))
        {
            cam->SetCameraPosition({ posArr[0], posArr[1], posArr[2] });
        }

        // 회전 (Pitch, Yaw)
        float pitch = cam->GetPitch();
        float yaw = cam->GetYaw();
        bool changed = false;
        if (ImGui::DragFloat("Pitch", &pitch, 0.1f, -89.0f, 89.0f)) changed = true;
        if (ImGui::DragFloat("Yaw", &yaw, 0.1f)) changed = true;

        if (changed) {
            cam->SetPitch(pitch);
            cam->SetYaw(yaw);
            // 카메라 내부 벡터(front 등) 업데이트는 Camera::Update()에서 수행됨
        }

        // 줌 (FOV)
        float zoom = cam->GetZoom();
        if (ImGui::SliderFloat("Zoom (FOV)", &zoom, 0.1f, 5.0f))
        {
            cam->SetZoom(zoom);
        }

        // Near / Far Plane
        float nearPlane = cam->GetNear();
        float farPlane = cam->GetFar();

        if (ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.01f, 10.0f)) cam->SetNear(nearPlane);
        if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, 10.0f, 10000.0f)) cam->SetFar(farPlane);

        ImGui::Separator();
        ImGui::Text("Viewport Settings (0.0 ~ 1.0)");

        // 뷰포트 (x, y, w, h)
        glm::vec4 vp = cam->GetRelativeViewport();
        float vpArr[4] = { vp.x, vp.y, vp.z, vp.w };

        if (ImGui::SliderFloat4("Rect (X, Y, W, H)", vpArr, 0.0f, 1.0f))
        {
            cam->SetRelativeViewport(vpArr[0], vpArr[1], vpArr[2], vpArr[3]);
        }

        // 빠른 프리셋 버튼
        if (ImGui::Button("Full Screen")) cam->SetRelativeViewport(0.0f, 0.0f, 1.0f, 1.0f);
        ImGui::SameLine();
        if (ImGui::Button("Left Half")) cam->SetRelativeViewport(0.0f, 0.0f, 0.5f, 1.0f);
        ImGui::SameLine();
        if (ImGui::Button("Right Half")) cam->SetRelativeViewport(0.5f, 0.0f, 0.5f, 1.0f);
    }

    ImGui::End();
    if (shouldDelete)
    {
        DeleteCamera(selectedCameraIndex);
    }
}
