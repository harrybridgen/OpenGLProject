#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <GL/gl3w.h>
#include "terrain.h"

struct Tree {
    glm::vec3 position;
    float scale;
    float rotationY; 
};

class TreeManager {
public:
    void Generate(int count, float worldSize, Terrain &terrain);
    void SetMeshes(const std::vector<float>& trunkVerts, const std::vector<float>& leafVerts);
    void SetupOpenGL();
    void LoadTextures(const char* trunkTex, const char* leafTex);
    void Render(glm::mat4& projection, glm::mat4& view,
        GLuint shaderProgram,
        glm::vec3& lightDir,
        glm::vec3& lightColor,
        glm::vec3& viewPos,
        const glm::mat4& lightSpaceMatrix,
        GLuint shadowMap,
        float sunElevation) const;


    void RenderShadow(GLuint shaderProgram) const;

private:
    std::vector<Tree> trees;

    std::vector<float> trunkVertices;
    std::vector<float> leafVertices;

    GLuint trunkVAO = 0, trunkVBO = 0;
    GLuint leafVAO = 0, leafVBO = 0;

    GLuint trunkTexture = 0;
    GLuint leafTexture = 0;

};
