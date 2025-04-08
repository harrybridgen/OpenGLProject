#pragma once

float Hash(glm::vec2 p) {
    return glm::fract(glm::sin(glm::dot(p, glm::vec2(127.1f, 311.7f))) * 53758.5453f);
}

float InterpolatedNoise(glm::vec2 p) {
    glm::vec2 i = glm::floor(p);
    glm::vec2 f = glm::fract(p);

    float a = Hash(i);
    float b = Hash(i + glm::vec2(1.0, 0.0));
    float c = Hash(i + glm::vec2(0.0, 1.0));
    float d = Hash(i + glm::vec2(1.0, 1.0));

    glm::vec2 u = f * f * (3.0f - 2.0f * f);

    return glm::mix(glm::mix(a, b, u.x), glm::mix(c, d, u.x), u.y);
}

float FractalNoise(glm::vec2 pos, int octaves = 4, float lacunarity = 2.0f, float gain = 0.5f) {
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float total = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += InterpolatedNoise(pos * frequency) * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return total;
}