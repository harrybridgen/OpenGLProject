#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Sun {
public:
    void Init(float worldSize);
    void Update(float deltaTime, const glm::vec3& targetPosition);

    const glm::vec3& GetDirection() const;
    const glm::vec3& GetColor() const;
    const glm::mat4& GetLightSpaceMatrix() const;
    float GetElevation() const;

private:
    float sunAngle = -glm::half_pi<float>();
    float worldSize;

    glm::vec3 direction;
    glm::vec3 color;
    glm::mat4 lightSpaceMatrix;
    float elevation;
};
