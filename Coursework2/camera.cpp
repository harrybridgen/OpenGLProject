#include "Camera.h"

Camera::Camera()
    : Target(10.0f, 10.0f, 10.0f),
    Distance(30.0f),
    Yaw(-90.0f),
    Pitch(20.0f),
    WorldUp(0.0f, 1.0f, 0.0f)
{
    UpdateVectors();
}

void Camera::UpdateVectors() {
    float yawRad = glm::radians(Yaw);
    float pitchRad = glm::radians(Pitch);

    glm::vec3 offset;
    offset.x = Distance * cos(pitchRad) * cos(yawRad);
    offset.y = Distance * sin(pitchRad);
    offset.z = Distance * cos(pitchRad) * sin(yawRad);

    Position = Target + offset;
    Front = glm::normalize(Target - Position);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::SetTarget(const glm::vec3& newTarget) {
    Target = newTarget;
    UpdateVectors();
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (Pitch > 65.0f) Pitch = 65.0f;
    if (Pitch < 15.0f) Pitch = 15.0f;

    UpdateVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    Distance -= yoffset * ZoomSpeed;
    if (Distance < 10.0f) Distance = 10.0f;
    if (Distance > 70.0f) Distance = 70.0f;
    UpdateVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Target, Up);
}
