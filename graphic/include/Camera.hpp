#pragma once
#include "glm.hpp"

enum class CameraMoveDir
{
	FORWARD,
	BACKWARD,
	UP,
	DOWN,
	LEFT,
	RIGHT
};

class Camera
{
public:
	Camera() noexcept;
	~Camera() = default;

	void Update();
	void Reset();

	void SetCameraPosition(glm::vec3 cameraPosition) noexcept;
	void SetViewSize(int width, int height) noexcept;
	void SetZoom(float amount) noexcept;
	void SetNear(float amount) noexcept { nearClip = amount; }
	void SetFar(float amount) noexcept { farClip = amount; }
	void SetPitch(float amount) noexcept { pitch = amount; }
	void SetYaw(float amount) noexcept { yaw = amount; }
	void SetCameraSensitivity(float amount) noexcept { cameraSensitivity = amount; }

	void SetTarget(glm::vec3 pos);
	void SetIsThirdPersonView(bool state) { isThirdPersonView = state; }
	void SetCameraDistance(float amount) noexcept { cameraDistance = amount; }
	void SetCameraOffset(glm::vec3 amount) noexcept { cameraOffset = amount; }

	glm::vec3 GetCameraPosition() const noexcept { return cameraPosition; }
	glm::mat4 GetViewMatrix() const { return view; }
	glm::mat4 GetProjectionMatrix() const { return projection; }
	float GetZoom() const { return zoom; }
	float GetPitch() const { return pitch; }
	float GetYaw() const { return yaw; }
	float GetNear() const { return nearClip; }
	float GetFar() const { return farClip; }
	bool GetIsThirdPersonView() const { return isThirdPersonView; }

	glm::vec3 GetFrontVector() const { return front; }
	glm::vec3 GetRightVector() const { return right; }
	glm::vec3 GetUpVector() const { return up; }
	glm::vec4 GetRelativeViewport() const { return relativeViewportRect; }

	void LookAt(glm::vec3 pos);
	void MoveCameraPos(CameraMoveDir dir, float speed);
	void UpdateCameraDirection(glm::vec2 dir);

	void SetRelativeViewport(float x, float y, float width, float height){	relativeViewportRect = { x, y, width, height }; }
private:
	glm::vec3 cameraPosition{ 0.0f, 0.0f, 0.0f };
	glm::vec2 cameraViewSize{ 0.0f, 0.0f };
	glm::vec4 relativeViewportRect{ 0.f, 0.f, 1.f, 1.f };
	float zoom = 1.0f;

	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	glm::vec3 worldUp{ 0.0f, 1.0f, 0.0f };
	glm::vec3 up{ 0.0f, 1.0f, 0.0f };
	glm::vec3 right{ 1.0f, 0.0f, 0.0f };
	glm::vec3 front{ 0.0f, 0.0f, 1.0f };

	float pitch = 0.0f;
	float yaw = 90.0f;
	float cameraSensitivity = 0.1f;

	float baseFov = 45.f;
	float nearClip = 0.1f;
	float farClip = 100.0f;

	bool isThirdPersonView = false;
	glm::vec3 cameraCenter{ 0.0f, 0.0f, 0.0f };
	glm::vec3 cameraOffset{ 0.0f, 0.0f, 0.0f };
	float cameraDistance = 5.0f;
};