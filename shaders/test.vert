#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 outColor;

layout(push_constant) uniform Constants {
    vec2 offset;
} push;

void main() {
    gl_Position = vec4(pos + vec3(push.offset, 0.0), 1.0);
    outColor = vec3(color);
}