#pragma once

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/quaternion.hpp>

class Transform
{
public:
    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetRotation() const { return rotation; }
    const glm::vec3& GetScale() const { return scale; }

    void SetPosition(const glm::vec3& position_) { position = position_; }
    void SetRotation(const glm::vec3& rotation_) { rotation = rotation_; }
    void SetScale(const glm::vec3& scale_) { scale = scale_; }

    void SetPosition(float x, float y, float z) { position.x = x; position.y = y; position.z = z; }
    void SetPositionX(float x) { position.x = x; }
    void SetPositionY(float y) { position.y = y; }
    void SetPositionZ(float z) { position.z = z; }

    void SetRotation(float x, float y, float z) { rotation.x = x; rotation.y = y; rotation.z = z; }
    void SetRotationX(float x) { rotation.x = x; }
    void SetRotationY(float y) { rotation.y = y; }
    void SetRotationZ(float z) { rotation.z = z; }

    void SetScale(float x, float y, float z) { scale.x = x; scale.y = y; scale.z = z; }
    void SetScaleX(float x) { scale.x = x; }
    void SetScaleY(float y) { scale.y = y; }
    void SetScaleZ(float z) { scale.z = z; }

    glm::mat4 GetModelMatrix() const
    {
        glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
        glm::quat rotQuat = glm::quat(glm::radians(rotation));
        glm::mat4 rotationMat = glm::mat4_cast(rotQuat);
        glm::mat4 translateMat = glm::translate(glm::mat4(1.0f), position);

        return translateMat * rotationMat * scaleMat;
    }
private:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
};
