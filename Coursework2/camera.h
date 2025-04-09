#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    void Init();
    void UpdateVectors();
    void SetTarget(const glm::vec3& newTarget);
    void ProcessMouseMovement(float xoffset, float yoffset);
    void ProcessMouseScroll(float yoffset);
    glm::mat4 GetViewMatrix() const;

    glm::vec3 GetPosition() const { return Position; }
    glm::vec3 GetTarget() const { return Target; }
	glm::vec3 GetUp() const { return Up; }
    float GetDistance() const { return Distance; }

    float GetYaw() const { return Yaw; }
    float GetPitch() const { return Pitch; }

    float MouseSensitivity = 0.2f;
    float ZoomSpeed = 2.0f;

private:
    glm::vec3 Target;
    float Distance;

    float Yaw;
    float Pitch;

    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
};
