#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 outColor;

layout(std140, binding = 0) uniform Constants {
    float elapsedTime;
};

void main() {
    vec2 offset = vec2(sin(elapsedTime), cos(elapsedTime));
    gl_Position = vec4(pos, 1.0) + vec4(offset, 0.0, 0.0);
    outColor = vec3(color);
}