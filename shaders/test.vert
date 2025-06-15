#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 uvIn;

layout(location = 0) out vec2 uvOut;

layout(std140, binding = 0) uniform Constants {
    mat4 model;
    mat4 view;
    mat4 projection;
    float elapsedTime;
};

void main() {
    gl_Position = projection * view * model * vec4(pos, 1.0);
    uvOut = uvIn;
}