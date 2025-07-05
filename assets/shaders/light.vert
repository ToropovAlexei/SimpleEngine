#version 460
#include "common.glsl"

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvIn;

void main() {
    gl_Position = projection * view * vec4(lightPosition + pos, 1.0);
}