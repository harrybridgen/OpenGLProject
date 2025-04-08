#ifndef WATER_H
#define WATER_H

#include <gl3w.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Water {
public:
    static void Init(float worldSize, float waterHeight);
    static void Render(const glm::mat4& projection, const glm::mat4& view, float sunElevation);
    static void Cleanup();

private:
    static GLuint VAO, VBO, shaderProgram, mvpLocation;
    static float height;
};

#endif
