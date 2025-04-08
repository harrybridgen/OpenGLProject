#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

uniform mat4 MVP;
uniform mat4 model;
uniform float time;
uniform bool isLeaf;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;

void main() {
    vec3 swayPosition = aPos;

    // Determine sway based on height
    float maxHeight = 5.0; 
    float heightFactor = clamp(aPos.y / maxHeight, 0.0, 1.0);
    float swayStrength = 0.1;
    float swaySpeed = 1.0;
    float swayOffset = sin(time * swaySpeed + aPos.x * 0.5 + aPos.z * 0.5) * swayStrength * heightFactor;

    swayPosition.x += swayOffset;

    vec4 worldPosition = model * vec4(swayPosition, 1.0);
    FragPos = worldPosition.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = MVP * vec4(swayPosition, 1.0);
}
