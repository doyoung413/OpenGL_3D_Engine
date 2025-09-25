#include "Camera.hpp"
#include "Engine.hpp"
#include "gtc/matrix_transform.hpp"

Camera::Camera() noexcept
{
    Reset();
}

void Camera::Update()
{
    // 카메라의 방향 벡터 계산
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));

    // 3인칭 뷰 모드 처리
    if (isThirdPersonView)
    {
        // 목표 지점(cameraCenter)에서 카메라 거리만큼 뒤로, 오프셋만큼 이동
        cameraPosition = cameraCenter - (front * cameraDistance) + cameraOffset;
        view = glm::lookAt(cameraPosition, cameraCenter, up);
    }
    else // 1인칭 뷰
    {
        view = glm::lookAt(cameraPosition, cameraPosition + (front), up);
    }

    // Projection 행렬 계산
    if (cameraViewSize.y > 0)
    {
        float aspectRatio = cameraViewSize.x / cameraViewSize.y;
        float fov = baseFov / zoom;
        projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
    }
}

void Camera::Reset()
{
    int winWidth = Engine::GetInstance().GetWindowWidth();
    int winHeight = Engine::GetInstance().GetWindowHeight();

    relativeViewportRect = { 0.0f, 0.0f, 1.f, 1.f };
    cameraViewSize = { static_cast<float>(winWidth), static_cast<float>(winHeight) };

    cameraPosition = { 0.0f, 0.0f, 0.0f };
    cameraCenter = { 0.0f, 0.0f, 0.0f };
    up = { 0.0f, 1.0f, 0.0f };
    worldUp = { 0.0f, 1.0f, 0.0f };
    front = { 0.0f, 0.0f, 1.f };

    pitch = 0.0f;
    yaw = 90.0f;
    zoom = 1.0f;
    isThirdPersonView = false;

    Update(); // 초기화된 값으로 행렬 업데이트
}

void Camera::SetCameraPosition(glm::vec3 cameraPosition_) noexcept
{
    cameraPosition = cameraPosition_;
}

void Camera::SetTarget(glm::vec3 pos)
{
    cameraCenter = pos;
}

void Camera::SetViewSize(int width, int height) noexcept
{
    cameraViewSize.x = static_cast<float>(width);
    cameraViewSize.y = static_cast<float>(height);
}

void Camera::SetZoom(float amount) noexcept
{
    zoom = amount;
    if (zoom < 0.1f) zoom = 0.1f;
}

void Camera::MoveCameraPos(CameraMoveDir dir, float speed)
{
    const glm::vec3 forward = front;

    switch (dir)
    {
    case CameraMoveDir::FORWARD:
        cameraPosition += forward * speed;
        break;
    case CameraMoveDir::BACKWARD:
        cameraPosition -= forward * speed;
        break;
    case CameraMoveDir::LEFT:
        cameraPosition -= right * speed;
        break;
    case CameraMoveDir::RIGHT:
        cameraPosition += right * speed;
        break;
    case CameraMoveDir::UP:
        cameraPosition += worldUp * speed;
        break;
    case CameraMoveDir::DOWN:
        cameraPosition -= worldUp * speed;
        break;
    }
}

// 특정 지점을 강제로 바라보게 함 (pitch/yaw 무시)
void Camera::LookAt(glm::vec3 pos)
{
    view = glm::lookAt(cameraPosition, pos, worldUp);
}

// 마우스 움직임에 따라 카메라의 Pitch와 Yaw를 갱신
void Camera::UpdateCameraDirection(glm::vec2 dir)
{
    // y값이 증가하면 위로 움직이므로 pitch는 감소
    float xoffset = dir.x * cameraSensitivity;
    float yoffset = dir.y * cameraSensitivity;

    yaw += xoffset;
    pitch -= yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}
