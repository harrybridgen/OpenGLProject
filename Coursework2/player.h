#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include "terrain.h"
#include "obj_loader.h"
#include <glm/gtc/constants.hpp>
#include <cmath>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

class Player {
public:
	void init(const glm::vec3& startPosition);
    void LoadModel(const std::string& path);
    void Render(GLuint shader, const glm::mat4& projection, const glm::mat4& view,
        const glm::vec3& lightDir, const glm::vec3& lightColor,
        const glm::vec3& viewPos, const glm::mat4& lightSpaceMatrix,
        GLuint shadowMap, float sunElevation);
    void SetTargetPosition(const glm::vec3& newTarget);
    void Update(float deltaTime, Terrain &terrain);
    glm::mat4 GetModelMatrix() const;
    glm::vec3 GetPosition() const;
    void RenderShadow(GLuint shader, const glm::mat4& lightSpaceMatrix);

private:
    glm::vec3 position;
    glm::vec3 targetPosition;
    float currentAngle;
    float targetAngle;
    float interpSpeed = 50.0f;

    GLuint VAO = 0, VBO = 0;
    std::vector<float> vertices;
};

