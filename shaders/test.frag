#version 460

layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform Constants {
    float elapsedTime;
};
layout(binding = 1) uniform sampler2D uTexture;

void main() {
    outColor = texture(uTexture, uv);
}