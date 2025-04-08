#include "water.h"
#include "shader.h"
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>

GLuint Water::VAO = 0;
GLuint Water::VBO = 0;
GLuint Water::shaderProgram = 0;
GLuint Water::mvpLocation = 0;
float Water::height = -2.0f;

void Water::Init(float worldSize, float waterHeight) {
    height = waterHeight;
    float vertices[] = {
        0.0f, height, 0.0f,
        worldSize, height, 0.0f,
        worldSize, height, worldSize,

        0.0f, height, 0.0f,
        worldSize, height, worldSize,
        0.0f, height, worldSize
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    shaderProgram = CompileShader("shaders/water.vert", "shaders/water.frag");
    mvpLocation = glGetUniformLocation(shaderProgram, "MVP");
}

void Water::Render(const glm::mat4& projection, const glm::mat4& view, float sunElevation)
{
    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -1.0, 0.0f));
    glm::mat4 mvp = projection * view * model;

    glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    float t = static_cast<float>(glfwGetTime());
    GLint timeLoc = glGetUniformLocation(shaderProgram, "time");
    GLint sunElevationLoc = glGetUniformLocation(shaderProgram, "sunElevation");
    glUniform1f(timeLoc, t);
    glUniform1f(sunElevationLoc, sunElevation);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Water::Cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}
