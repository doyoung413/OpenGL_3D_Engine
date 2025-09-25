#include "Camera.hpp"
#include "Engine.hpp"
#include "gtc/matrix_transform.hpp"

Camera::Camera() noexcept
{
    Reset();
}

void Camera::Update()
{
    // ī�޶��� ���� ���� ���
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(direction);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));

    // 3��Ī �� ��� ó��
    if (isThirdPersonView)
    {
        // ��ǥ ����(cameraCenter)���� ī�޶� �Ÿ���ŭ �ڷ�, �����¸�ŭ �̵�
        cameraPosition = cameraCenter - (front * cameraDistance) + cameraOffset;
        view = glm::lookAt(cameraPosition, cameraCenter, up);
    }
    else // 1��Ī ��
    {
        view = glm::lookAt(cameraPosition, cameraPosition + (front), up);
    }

    // Projection ��� ���
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

    Update(); // �ʱ�ȭ�� ������ ��� ������Ʈ
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

// Ư�� ������ ������ �ٶ󺸰� �� (pitch/yaw ����)
void Camera::LookAt(glm::vec3 pos)
{
    view = glm::lookAt(cameraPosition, pos, worldUp);
}

// ���콺 �����ӿ� ���� ī�޶��� Pitch�� Yaw�� ����
void Camera::UpdateCameraDirection(glm::vec2 dir)
{
    // y���� �����ϸ� ���� �����̹Ƿ� pitch�� ����
    float xoffset = dir.x * cameraSensitivity;
    float yoffset = dir.y * cameraSensitivity;

    yaw += xoffset;
    pitch -= yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}
