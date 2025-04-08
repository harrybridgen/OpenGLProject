#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform float time;
uniform float sunElevation;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(321, 123))) * 12345.12345);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
}

float shimmer(vec2 uv, float t) {
    float n1 = noise(uv * 10.0 + vec2(t * 0.3, t * 0.2));
    float n2 = noise(uv * 20.0 - vec2(t * 0.4, t * 0.1));
    float n3 = noise(uv * 5.0 + vec2(-t * 0.2, t * 0.25));
    return (n1 + n2 * 0.5 + n3 * 0.25) * 0.1;
}

void main() {
    float sunlight = clamp(-sunElevation, 0.0, 1.0);

    vec3 nightColor = vec3(0.005, 0.01, 0.02); 
    vec3 dayColor = vec3(0.0, 0.45, 0.75);
    vec3 baseColor = mix(nightColor, dayColor, sunlight);

    float brightness = shimmer(TexCoord, time * 2.0f);
    vec3 finalColor = baseColor + vec3(brightness);

    FragColor = vec4(finalColor, 0.35); 
}
