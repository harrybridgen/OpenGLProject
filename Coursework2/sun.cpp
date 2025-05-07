#include "sun.h"
#include <glm/gtc/matrix_transform.hpp>

void Sun::Init(float worldSize) {
    this->worldSize = worldSize;
}

void Sun::Update(float deltaTime, const glm::vec3& targetPosition) {

	sunAngle += deltaTime * sunSpeed;

    if (sunAngle > 0.5f)
        sunAngle = -glm::pi<float>() - 0.5f;

    elevation = glm::sin(sunAngle);
    float horizontal = glm::cos(sunAngle);

    direction = glm::normalize(glm::vec3(horizontal, elevation, -0.7f));

    float daylight = glm::clamp(glm::dot(direction, glm::vec3(0, -1, 0)), 0.0f, 1.0f);
    color = glm::mix(glm::vec3(1.0f, 0.5f, 0.2f), glm::vec3(1.0f), daylight);

    glm::vec3 lightPos = targetPosition - direction * 200.0f;
    glm::mat4 lightProj = glm::ortho(
        -worldSize * 0.6f, worldSize * 0.6f,
        -worldSize * 0.6f, worldSize * 0.6f,
        10.0f, worldSize
    );

    glm::mat4 lightView = glm::lookAt(lightPos, targetPosition, glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProj * lightView;
}

const glm::vec3& Sun::GetDirection() const { return direction; }
const glm::vec3& Sun::GetColor() const { return color; }
const glm::mat4& Sun::GetLightSpaceMatrix() const { return lightSpaceMatrix; }
float Sun::GetElevation() const { return elevation; }
