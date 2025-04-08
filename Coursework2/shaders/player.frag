#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightColor;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform mat4 lightSpaceMatrix;
uniform sampler2D shadowMap;
uniform float sunElevation;

out vec4 FragColor;

float ShadowCalc(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.001);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
            shadow += currentDepth - bias > texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r ? 1.0 : 0.0;

    shadow /= 9.0;
    if (projCoords.z > 1.0) shadow = 0.0;
    return shadow;
}

void main() {
    vec3 baseColor = vec3(0.5);
    vec3 norm = -normalize(Normal);
    vec3 lightDirNorm = normalize(lightDir);
    vec3 ambient = 0.2 * lightColor;

    float diff = max(dot(norm, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor;

    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    float shadow = ShadowCalc(fragPosLightSpace, norm, lightDirNorm);

    float sunlight = clamp(-sunElevation, 0.0, 1.0);

    vec3 lighting = ambient * baseColor;
    lighting += (1.0 - shadow) * diffuse * baseColor * sunlight;

    FragColor = vec4(lighting, 1.0);
}
