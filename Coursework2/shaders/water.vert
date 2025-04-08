#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 MVP;
uniform float time;

out vec2 TexCoord;

void main() {
    vec3 pos = aPos;

    pos.y += sin(pos.x * 0.1 + time) * 0.2;
    pos.y += cos(pos.z * 0.1 + time) * 0.2;

    TexCoord = pos.xz * 0.05;

    gl_Position = MVP * vec4(pos, 1.0);
}
