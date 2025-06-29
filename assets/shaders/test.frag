#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform Constants {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    vec3 lightPosition;
    vec3 lightColor;
    float elapsedTime;
};
layout(binding = 1) uniform sampler2D uTexture;

void main() {
    vec3 norm = normalize(normal);
    
    // Ambient
    const float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 lightDir = normalize(lightPosition - pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular (Blinn-Phong)
    vec3 viewDir = normalize(cameraPos - pos); // Теперь используем cameraPos
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfDir), 0.0), 32.0);
    vec3 specular = spec * lightColor;

    // Combine lighting and texture
    vec4 lighting = vec4(ambient + diffuse + specular, 1.0);
    vec4 texColor = texture(uTexture, uv);

    outColor = lighting * texColor;
}