#include "terrain.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <cmath>
#include "texture.h"
#include "shader.h"
#include <iostream>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include "noise.h"

void Terrain::Init(int tilesX, int tilesZ) {
    this->tilesX = tilesX;
    this->tilesZ = tilesZ;

    cliffTexture = LoadTexture("textures/cliff_side_diff_4k.jpg");
    grassTexture = LoadTexture("textures/rocky_terrain_02_diff_4k.jpg");
    riverbedTexture = LoadTexture("textures/sandy_gravel_02_diff_4k.jpg");

    std::vector<float> tileMesh = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };

    terrainMesh = BuildTerrainMesh(tileMesh, tilesX, tilesZ);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, terrainMesh.size() * sizeof(float), terrainMesh.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    shaderProgram = CompileShader("shaders/tile.vert", "shaders/tile.frag");
}

void Terrain::Render(const glm::mat4& projection, const glm::mat4& view, const glm::vec3& cameraPos,
    const glm::vec3& lightDir, const glm::vec3& lightColor, const glm::mat4& lightSpaceMatrix,
    GLuint shadowMap, float sunElevation) {

    glUseProgram(shaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(cameraPos));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform1f(glGetUniformLocation(shaderProgram, "sunElevation"), -sunElevation);
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), 4.0f);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cliffTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "cliffTex"), 0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, grassTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "grassTex"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, riverbedTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "riverbedTex"), 3);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 1);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, terrainMesh.size() / 8);
}

void Terrain::Cleanup() {
    glDeleteTextures(1, &cliffTexture);
    glDeleteTextures(1, &grassTexture);
    glDeleteTextures(1, &riverbedTexture);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);
}
bool Terrain::RaycastToTerrain(const glm::vec3& origin, const glm::vec3& direction, glm::vec3& hitPoint) {
    float t = 0.0f;
    const float tStep = 1.0f;
    const float tMax = 500.0f;

    for (int i = 0; i < 512 && t < tMax; ++i) {
        glm::vec3 pos = origin + direction * t;
        float terrainY = GetTileHeight(pos.x, pos.z);

        if (pos.y <= terrainY) {
            hitPoint = pos;
            return true;
        }

        t += tStep;
    }

    return false;
}

float Terrain::GetTileHeight(float x, float z) {

    glm::vec2 pos = glm::vec2(x, z) * 0.004f;
    float baseNoise = FractalNoise(pos, 5, 2.0f, 0.55f);


    float centered = baseNoise - 0.5f;

    float elevation = 0.0f;
    if (centered > 0.0f) {
        elevation = glm::smoothstep(0.0f, 0.8f, centered) * 40.0f; 
    }
    else {
        elevation = -glm::smoothstep(0.0f, 0.8f, -centered) * 40.0f; 
    }


    glm::vec2 microPos = glm::vec2(x, z) * 0.05f;
    float microNoise = InterpolatedNoise(microPos) * 0.5f - 0.25f;

    return elevation + microNoise;
}

std::vector<float> Terrain::BuildTerrainMesh(const std::vector<float>& tileVerts, int tilesX, int tilesZ) {
    std::vector<float> terrainMesh;

    for (int x = 0; x < tilesX; ++x) {
        for (int z = 0; z < tilesZ; ++z) {
            for (size_t i = 0; i < tileVerts.size(); i += 9) {
                glm::vec3 p0(tileVerts[i], tileVerts[i + 1], tileVerts[i + 2]);
                glm::vec3 p1(tileVerts[i + 3], tileVerts[i + 4], tileVerts[i + 5]);
                glm::vec3 p2(tileVerts[i + 6], tileVerts[i + 7], tileVerts[i + 8]);

                glm::vec3 worldP[3];
                for (int j = 0; j < 3; ++j) {
                    glm::vec3 lp = (j == 0 ? p0 : (j == 1 ? p1 : p2));
                    float worldX = lp.x + x;
                    float worldZ = lp.z + z;
                    float height = GetTileHeight(worldX, worldZ);
                    worldP[j] = glm::vec3(worldX, lp.y + height - 0.5f, worldZ);
                }

                glm::vec3 edge1 = worldP[1] - worldP[0];
                glm::vec3 edge2 = worldP[2] - worldP[0];
                glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));


                for (int j = 0; j < 3; ++j) {
                    terrainMesh.push_back(worldP[j].x);
                    terrainMesh.push_back(worldP[j].y);
                    terrainMesh.push_back(worldP[j].z);
                    terrainMesh.push_back(normal.x);
                    terrainMesh.push_back(normal.y);
                    terrainMesh.push_back(normal.z);
                    float u = worldP[j].x / (float)tilesX;
                    float v = worldP[j].z / (float)tilesZ;
                    terrainMesh.push_back(u);
                    terrainMesh.push_back(v);
                }
            }
        }
    }

    return terrainMesh;
}

