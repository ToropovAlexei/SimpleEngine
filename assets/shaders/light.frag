#version 460
#include "common.glsl"

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(lightColor, 1.0);
}