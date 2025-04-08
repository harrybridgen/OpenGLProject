#include "player.h"
#include "terrain.h"

void Player::init(const glm::vec3& startPosition) {
	position = startPosition;
	targetPosition = startPosition;
	currentAngle = 0.0f;
	targetAngle = 0.0f;
}

void Player::SetTargetPosition(const glm::vec3& newTarget) {
    targetPosition = newTarget;
}
void Player::LoadModel(const std::string& path) {
    vertices = LoadMyObjWithNormals(path);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0); 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); 
    glEnableVertexAttribArray(1);

}
void Player::RenderShadow(GLuint shader, const glm::mat4& lightSpaceMatrix) {
    glUseProgram(shader);

    glm::mat4 model = GetModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
}

void Player::Render(GLuint shader, const glm::mat4& projection, const glm::mat4& view,
    const glm::vec3& lightDir, const glm::vec3& lightColor,
    const glm::vec3& viewPos, const glm::mat4& lightSpaceMatrix,
    GLuint shadowMap, float sunElevation) {
    glUseProgram(shader);

    glm::mat4 model = GetModelMatrix();
    glm::mat4 mvp = projection * view * model;

    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(glGetUniformLocation(shader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glUniform3fv(glGetUniformLocation(shader, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(viewPos));

    glUniform1f(glGetUniformLocation(shader, "sunElevation"), sunElevation);


    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(glGetUniformLocation(shader, "shadowMap"), 1);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);
}

void Player::Update(float deltaTime, Terrain& terrain) {
    glm::vec3 toTarget = targetPosition - position;
    float distance = glm::length(toTarget);

    if (distance > 0.01f) {
        glm::vec3 dir = glm::normalize(toTarget);
        float moveStep = interpSpeed * deltaTime;

        if (distance <= moveStep) {
            position = targetPosition;
        } else {
            position += dir * moveStep;
        }

        targetAngle = atan2(dir.x, dir.z);
    }

    position.y = terrain.GetTileHeight(position.x, position.z) - 0.5f;

    float delta = targetAngle - currentAngle;
    delta = fmod(delta + glm::pi<float>(), glm::two_pi<float>()) - glm::pi<float>();
    currentAngle += delta * 10.0f * deltaTime;
}



glm::mat4 Player::GetModelMatrix() const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::rotate(model, currentAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.2f));
    return model;
}

glm::vec3 Player::GetPosition() const {
    return position;
}
