#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "shader.h"
#include "obj_loader.h"
#include "camera.h"
#include "terrain.h"
#include "player.h"
#include "tree.h"
#include "water.h"
#include "stb_image.h"
#include "sun.h"

const unsigned int WIDTH = 1400;
const unsigned int HEIGHT = 800;
const unsigned int NUM_TREES = 70;
const int WORLD_SIZE = 500;

bool firstMouse = true;

GLuint shadowFBO, shadowMap;
const GLuint SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;

glm::mat4 lightSpaceMatrix;

struct MouseContext {
    Camera* camera;
    Terrain* terrain;
    Player* player;
};


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        float x = (2.0f * xpos) / width - 1.0f;
        float y = 1.0f - (2.0f * ypos) / height;
        glm::vec4 ray_clip(x, y, -1.0f, 1.0f);

        MouseContext* ctx = reinterpret_cast<MouseContext*>(glfwGetWindowUserPointer(window));
        Camera* cam = ctx->camera;
        Terrain* terrain = ctx->terrain;
        Player* player = ctx->player;

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
        glm::mat4 view = cam->GetViewMatrix();

        glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
        glm::vec3 ray_world = glm::normalize(glm::vec3(glm::inverse(view) * ray_eye));
        glm::vec3 origin = cam->GetPosition();
        glm::vec3 dir = glm::normalize(ray_world);

        glm::vec3 hitPoint;
        if (terrain->RaycastToTerrain(origin, dir, hitPoint)) {
            float finalY = terrain->GetTileHeight(hitPoint.x, hitPoint.z);
            player->SetTargetPosition(glm::vec3(hitPoint.x, finalY, hitPoint.z));
        }
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        firstMouse = true;
    }
}



void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static double lastX = WIDTH / 2.0f;
    static double lastY = HEIGHT / 2.0f;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
        return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = float(xpos - lastX);
    float yoffset = float(ypos - lastY);
    lastX = xpos;
    lastY = ypos;

    MouseContext* ctx = reinterpret_cast<MouseContext*>(glfwGetWindowUserPointer(window));
    if (!ctx || !ctx->camera) return;

    ctx->camera->ProcessMouseMovement(xoffset, yoffset);
}



void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    MouseContext* ctx = reinterpret_cast<MouseContext*>(glfwGetWindowUserPointer(window));
    if (!ctx || !ctx->camera) return;

    ctx->camera->ProcessMouseScroll(float(yoffset));
}


GLuint postFBO, postColorTex, postDepthTex;

void SetupPostProcessingFrameBuffer() {
    glGenFramebuffers(1, &postFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postFBO);

    // Color attachment
    glGenTextures(1, &postColorTex);
    glBindTexture(GL_TEXTURE_2D, postColorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postColorTex, 0);

    // Depth attachment
    glGenTextures(1, &postDepthTex);
    glBindTexture(GL_TEXTURE_2D, postDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WIDTH, HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, postDepthTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Post-processing framebuffer not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint quadVAO, quadVBO;
void SetupFullscreenQuad() {
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}
void RenderFogPostProcessing(GLuint fogShader, GLuint postColorTex, GLuint postDepthTex,
    glm::vec3 fogColor, glm::vec3 cameraPos,
    const glm::mat4& projection, const glm::mat4& view) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, WIDTH, HEIGHT);

    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(fogShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postColorTex);
    glUniform1i(glGetUniformLocation(fogShader, "scene"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, postDepthTex);
    glUniform1i(glGetUniformLocation(fogShader, "depthMap"), 1);

    glUniform3fv(glGetUniformLocation(fogShader, "fogColor"), 1, glm::value_ptr(fogColor));
    glUniform3fv(glGetUniformLocation(fogShader, "camPos"), 1, glm::value_ptr(cameraPos));

    glm::mat4 invProj = glm::inverse(projection);
    glm::mat4 invView = glm::inverse(view);

    glUniformMatrix4fv(glGetUniformLocation(fogShader, "invProj"), 1, GL_FALSE, glm::value_ptr(invProj));
    glUniformMatrix4fv(glGetUniformLocation(fogShader, "invView"), 1, GL_FALSE, glm::value_ptr(invView));

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glEnable(GL_DEPTH_TEST);
}

void SetupShadowFramebuffer(GLuint& shadowFBO, GLuint& shadowMap, GLuint width, GLuint height) {
    glGenFramebuffers(1, &shadowFBO);
    glGenTextures(1, &shadowMap);

    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main() {
    std::srand(static_cast<unsigned int>(std::time(0)));

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "RunEscape", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    if (gl3wInit()) return -1;

	// OpenGL options
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //init shadow frame buffer
	SetupShadowFramebuffer(shadowFBO, shadowMap, SHADOW_WIDTH, SHADOW_HEIGHT);

    // init camera
    Camera camera;
	camera.Init();

    //init terrain
    Terrain terrain;
    terrain.Init(WORLD_SIZE, WORLD_SIZE);

    //init water
	Water water = Water();
    water.Init(WORLD_SIZE, 0.0f);

    //init terrain
    TreeManager treeManager;
    treeManager.Generate(NUM_TREES, WORLD_SIZE, terrain);

    //init player
    Player player;
    player.init(glm::vec3(WORLD_SIZE / 2, 0.0f, WORLD_SIZE / 2));
    player.LoadModel("objs/human.obj");

    //init sun
    Sun sun;
    sun.Init(WORLD_SIZE);

    //init input call backs
    MouseContext mouseContext = { &camera, &terrain , &player};
    glfwSetWindowUserPointer(window, &mouseContext);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // init shaders
    GLuint tileShader = CompileShader("shaders/tile.vert", "shaders/tile.frag");
    GLuint playerShader = CompileShader("shaders/player.vert", "shaders/player.frag");
    GLuint treeShader = CompileShader("shaders/tree.vert", "shaders/tree.frag");
    GLuint shadowShader = CompileShader("shaders/shadow_depth.vert", "shaders/shadow_depth.frag");
    GLuint fogShader = CompileShader("shaders/fog_post.vert", "shaders/fog_post.frag");

    // init trees
    std::vector<MeshSegment> segments = LoadMeshByMaterial("objs/Tree.obj");
    std::vector<float> trunkVerts;
    std::vector<float> leafVerts;
    for (const auto& seg : segments) {
        if (seg.materialName == "Trank_bark") {
            trunkVerts = seg.vertices;
        }
        else if (seg.materialName == "polySurface1SG1") {
            leafVerts = seg.vertices;
        }
        else {
            std::cerr << "Unknown material: " << seg.materialName << "\n";
        }
    }
    treeManager.SetMeshes(trunkVerts, leafVerts);
    treeManager.LoadTextures("objs/bark_0021.jpg", "objs/DB2X2_L01.png");
    treeManager.SetupOpenGL();

	//init prost processing
    SetupPostProcessingFrameBuffer();
    SetupFullscreenQuad();

	//init delta time
    float lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        // calc delta time df
        float currentFrameTime = glfwGetTime();
        float dt = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        
        glfwPollEvents();

        // update player
        player.Update(dt, terrain);

        // update camera
        camera.SetTarget(player.GetPosition() + glm::vec3(0.0f, 2.6f, 0.0f));
		camera.UpdateVectors();

		//update sun & related variables
		sun.Update(dt, player.GetPosition());
        glm::vec3 lightDir = sun.GetDirection();
        glm::vec3 lightColor = sun.GetColor();
        float sunElevation = sun.GetElevation();
        glm::mat4 lightSpaceMatrix = sun.GetLightSpaceMatrix();

		// update shadow map
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(shadowShader);
        glUniformMatrix4fv(glGetUniformLocation(shadowShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
        treeManager.RenderShadow(shadowShader);
        player.RenderShadow(shadowShader, lightSpaceMatrix);

		// change to post processing framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, postFBO);
        glViewport(0, 0, WIDTH, HEIGHT);  

        // render sky sky
        float t = glm::clamp(sunElevation, -1.0f, 1.0f);
        float nightAmount = glm::clamp(glm::smoothstep(-0.9f, 0.0f, t), 0.0f, 1.0f);
        glm::vec3 nightColor = glm::vec3(0.05f, 0.05f, 0.1f);
        glm::vec3 dayColor = glm::vec3(0.5f, 0.7f, 1.0f);
        glm::vec3 skyColor;
        skyColor = glm::mix(dayColor, nightColor, nightAmount);
        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// update projection and view matrices
        float nearPlane = 0.1f;
        float farPlane = WORLD_SIZE;
		glm::vec3 cameraPos = camera.GetPosition();
		glm::vec3 cameraTarget = camera.GetTarget();
		glm::vec3 cameraUp = camera.GetUp();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / HEIGHT, nearPlane, farPlane);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

        // render floor
        terrain.Render(projection, view, cameraPos, lightDir, lightColor, lightSpaceMatrix, shadowMap, sunElevation);

        // render player
        player.Render(playerShader, projection, view, lightDir, lightColor, cameraPos, lightSpaceMatrix, shadowMap, sunElevation);


        // render trees
        treeManager.Render(projection, view, treeShader, lightDir, lightColor, cameraPos, lightSpaceMatrix, shadowMap, sunElevation);

        // render water
        water.Render(projection, view, sunElevation);

		// render post processing
        glm::vec3 fogDay = glm::vec3(0.6f, 0.7f, 0.8f);
        glm::vec3 fogNight = glm::vec3(0.05f, 0.06f, 0.08f);
        glm::vec3 fogColor = glm::mix(fogDay, fogNight, nightAmount);
        RenderFogPostProcessing(fogShader, postColorTex, postDepthTex, fogColor, cameraPos, projection, view);

        glfwSwapBuffers(window);
		
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    water.Cleanup();
    return 0;
}