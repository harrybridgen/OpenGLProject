#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/gl3w.h>

class Terrain {
public:
    void Init(int tilesX, int tilesZ);
    void Render(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos, const glm::vec3& lightDir,
        const glm::vec3& lightColor, const glm::mat4& lightSpaceMatrix, GLuint shadowMap, float sunElevation);
    void Cleanup();

    float GetTileHeight(float x, float z);
    std::vector<float> BuildTerrainMesh(const std::vector<float>& tileVerts, int tilesX, int tilesZ);
    bool RaycastToTerrain(const glm::vec3& origin, const glm::vec3& direction, glm::vec3& hitPoint);


private:
    std::vector<float> terrainMesh;
    GLuint VAO = 0, VBO = 0;
    GLuint cliffTexture = 0, grassTexture = 0, riverbedTexture = 0;
    GLuint shaderProgram = 0;
    int tilesX = 0, tilesZ = 0;
};
