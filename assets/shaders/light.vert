#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvIn;

layout(std140, binding = 0) uniform Constants {
    mat4 view;
    mat4 projection;
    vec3 cameraPos;
    vec3 lightPosition;
    vec3 lightColor;
    float elapsedTime;
};

void main() {
    gl_Position = projection * view * vec4(lightPosition + pos, 1.0);
}