#define STB_IMAGE_IMPLEMENTATION

#include "tree.h"
#include "terrain.h"
#include "stb_image.h"
#include <gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdlib>
#include <iostream>
#include <cmath>



void TreeManager::Generate(int count, float worldSize, Terrain &terrain) {
    trees.clear();
    int attempts = 0;
    int maxAttempts = count * 10;

    while (trees.size() < count && attempts < maxAttempts) {
        attempts++;

        float x = static_cast<float>(rand()) / RAND_MAX * worldSize;
        float z = static_cast<float>(rand()) / RAND_MAX * worldSize;
        float y = terrain.GetTileHeight(x, z);

		// if too low (as in the water plane) skip this tree
        if (y < -1.5f)
            continue;

        // sink into ground a bit to ensure clip into terrain
		y -= 0.5f;

        Tree tree;
        tree.position = glm::vec3(x, y, z);
        tree.scale = 6.0f + static_cast<float>(rand() % 100) / 40.0f;
        tree.rotationY = static_cast<float>(rand()) / RAND_MAX * glm::two_pi<float>();
        trees.push_back(tree);
    }

    if (trees.size() < count) {
        std::cout << "Only placed " << trees.size() << " trees out of " << count << " after " << attempts << " attempts.\n";
    }
}

void TreeManager::LoadTextures(const char* trunkTex, const char* leafTex) {
    // Trunk
    glGenTextures(1, &trunkTexture);
    glBindTexture(GL_TEXTURE_2D, trunkTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(trunkTex, &w, &h, &ch, 0);
    if (data) {
        GLenum format = ch == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }

    // Leaves
    glGenTextures(1, &leafTexture);
    glBindTexture(GL_TEXTURE_2D, leafTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    data = stbi_load(leafTex, &w, &h, &ch, 0);
    if (data) {
        GLenum format = ch == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
}

void TreeManager::SetMeshes(const std::vector<float>& trunkVerts, const std::vector<float>& leafVerts) {
    trunkVertices = trunkVerts;
    leafVertices = leafVerts;
}

void TreeManager::SetupOpenGL() {
    glGenVertexArrays(1, &trunkVAO);
    glGenBuffers(1, &trunkVBO);
    glGenVertexArrays(1, &leafVAO);
    glGenBuffers(1, &leafVBO);

    // Trunk
    glBindVertexArray(trunkVAO);
    glBindBuffer(GL_ARRAY_BUFFER, trunkVBO);
    glBufferData(GL_ARRAY_BUFFER, trunkVertices.size() * sizeof(float), trunkVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);               // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); // TexCoord
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); // Normal
    glEnableVertexAttribArray(2);

    // Leaves
    glBindVertexArray(leafVAO);
    glBindBuffer(GL_ARRAY_BUFFER, leafVBO);
    glBufferData(GL_ARRAY_BUFFER, leafVertices.size() * sizeof(float), leafVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

}

void TreeManager::RenderShadow(GLuint shaderProgram) const {
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");

    for (const auto& tree : trees) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), tree.position);
        model = glm::rotate(model, tree.rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(tree.scale));

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(trunkVAO);
        glDrawArrays(GL_TRIANGLES, 0, trunkVertices.size() / 8);

        glBindVertexArray(leafVAO);
        glDrawArrays(GL_TRIANGLES, 0, leafVertices.size() / 8);
    }
}

void TreeManager::Render(glm::mat4& projection, glm::mat4& view,
    GLuint shaderProgram,
    glm::vec3& lightDir,
    glm::vec3& lightColor,
    glm::vec3& viewPos,
    const glm::mat4& lightSpaceMatrix,
    GLuint shadowMap,
    float sunElevation) const
    const{
    glUseProgram(shaderProgram);
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), glfwGetTime());

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(glGetUniformLocation(shaderProgram, "shadowMap"), 1);
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightDir"), 1, glm::value_ptr(lightDir));
    glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
    glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(viewPos));
    glUniform1f(glGetUniformLocation(shaderProgram, "sunElevation"), sunElevation);glUniform1f(glGetUniformLocation(shaderProgram, "texPixelSize"), 0.05f);
    GLuint mvpLoc = glGetUniformLocation(shaderProgram, "MVP");
    GLuint texLoc = glGetUniformLocation(shaderProgram, "treeTexture");
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    
    for (const auto& tree : trees) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), tree.position);
        model = glm::rotate(model, tree.rotationY, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(tree.scale));
        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        
        GLint isLeafLoc = glGetUniformLocation(shaderProgram, "isLeaf");

        // Draw trunk
        glUniform1i(isLeafLoc, GL_FALSE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, trunkTexture);
        glUniform1i(texLoc, 0);
        glBindVertexArray(trunkVAO);
        glDrawArrays(GL_TRIANGLES, 0, trunkVertices.size() / 8);

        // Draw leaves
        glUniform1i(isLeafLoc, GL_TRUE);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, leafTexture);
        glUniform1i(texLoc, 0);
        glBindVertexArray(leafVAO);
        glDrawArrays(GL_TRIANGLES, 0, leafVertices.size() / 8);

    }
}

