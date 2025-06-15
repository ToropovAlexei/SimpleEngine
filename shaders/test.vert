#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvIn;

layout(location = 0) out vec2 uvOut;

layout(std140, binding = 0) uniform Constants {
    float elapsedTime;
};

void main() {
    vec2 offset = vec2(sin(elapsedTime), cos(elapsedTime));
    gl_Position = (vec4(pos, 1.0) + vec4(offset, 0.0, 0.0)) * vec4(1.0, -1.0, 1.0, 1.0);
    uvOut = uvIn;
}