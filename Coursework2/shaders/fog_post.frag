#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D depthMap;
uniform vec3 fogColor;
uniform vec3 camPos;
uniform mat4 invProj;
uniform mat4 invView;

void main() {
    float rawDepth = texture(depthMap, TexCoords).r;

  // Uncomment if you want to skip fog for sky
  //  if (rawDepth >= 1.0) {
  //    FragColor = texture(scene, TexCoords); // Background, skip fog
  //    return;
  //  }

    vec4 clipSpace = vec4(TexCoords * 2.0 - 1.0, rawDepth * 2.0 - 1.0, 1.0);

    vec4 viewPos = invProj * clipSpace;
    viewPos /= viewPos.w;

    vec4 worldPos = invView * viewPos;

    float dist = length(camPos - worldPos.xyz);

    float fogDensity = 0.002;
    float fogFactor = 1.0 - exp(-dist * fogDensity);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 sceneColor = texture(scene, TexCoords).rgb;
    vec3 finalColor = mix(sceneColor, fogColor, fogFactor);

    FragColor = vec4(finalColor, 1.0);
}
