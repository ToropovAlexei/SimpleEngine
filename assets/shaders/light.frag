#version 460

layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform Constants {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    vec3 lightPosition;
    vec3 lightColor;
    float elapsedTime;
};

void main() {
    outColor = vec4(lightColor, 1.0);
}