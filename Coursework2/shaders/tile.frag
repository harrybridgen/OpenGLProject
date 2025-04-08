#version 330 core

in vec3 worldPosition;
in vec3 Normal;
in vec3 FragPos;
in vec2 UV;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPos;

uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform float sunElevation;
uniform sampler2D cliffTex;
uniform sampler2D grassTex;
uniform sampler2D riverbedTex;



out vec4 FragColor;

float hash(vec2 p) {
    return fract(sin(dot(p, vec2(341.5, 643.02))) * 19281.827182);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) +
           (c - a) * u.y * (1.0 - u.x) +
           (d - b) * u.x * u.y;
}

float ShadowCalc(vec4 fragPosLightSpace) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    float bias = max(0.001 * (1.0 - dot(normalize(Normal), normalize(lightDir))), 0.001);


    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;


    if (projCoords.z > 1.0)
        shadow = 0.0;
    float sunShadowFactor = clamp(sunElevation, 0.0, 1.0);
    shadow *= sunShadowFactor;
    return shadow;
}

void main() {
    vec3 sand = texture(riverbedTex, UV * 24.0).rgb;
    vec3 grass = texture(grassTex, UV * 24.0).rgb;
    vec3 stone = texture(cliffTex, UV * 24.0).rgb;


    // Grass/sand based on height
    float heightFactor = smoothstep(-4.0, -1.0, worldPosition.y);

    // Noise for color variation
    float variation = noise(worldPosition.xz * 0.5) * 0.05;
    vec3 baseColor = mix(sand, grass, heightFactor) + variation;

    // Steepness detection
    float steepness = smoothstep(0.99, 0.7, abs(Normal.y));

    // Blend in stone on steep slopes
    baseColor = mix(baseColor, stone, steepness);

    // Lighting
    vec3 ambient = 0.2 * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    float daylightFactor = clamp(sunElevation, 0.0, 1.0);
    vec3 diffuse = diff * lightColor * daylightFactor;


    // Shadows
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    float shadow = ShadowCalc(fragPosLightSpace);

    vec3 lighting = (ambient + (1.0 - shadow) * diffuse) * baseColor;

    FragColor = vec4(lighting, 1.0);
}

