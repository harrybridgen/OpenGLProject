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
uniform float shininess;

out vec4 FragColor;


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
    vec3 baseColor = mix(sand, grass, heightFactor);

    // Steepness detection and blend in stone on steep slopes
    float steepness = smoothstep(0.99, 0.7, abs(Normal.y));
    baseColor = mix(baseColor, stone, steepness);

    // Lighting
    vec3 ambient = 0.2 * lightColor;
    vec3 norm = normalize(Normal);
    vec3 lightDirNorm = normalize(lightDir);
    float diff = max(dot(norm, lightDirNorm), 0.0);
    float daylightFactor = clamp(sunElevation, 0.0, 1.0);
    vec3 diffuse = diff * lightColor * daylightFactor;

    // Specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 halfwayDir = normalize(lightDirNorm - viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), shininess);
    vec3 specular = spec * lightColor * daylightFactor * 0.5;

    // Shadows
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    float shadow = ShadowCalc(fragPosLightSpace);

    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * baseColor;
    
    //vec3 lighting = specular;
    FragColor = vec4(lighting, 1.0);


}

