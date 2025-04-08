#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

uniform mat4 MVP;
uniform mat4 model;

out vec3 worldPosition;
out vec3 Normal;
out vec3 FragPos;
out vec2 UV;

void main() {
    worldPosition = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPos = worldPosition.xyz;
    UV = aUV;
    gl_Position = MVP * vec4(aPos, 1.0);
}
