#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 fragCoord;
layout (location = 1) flat in uint fragColor;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D outputImage1;
layout(binding = 2) uniform sampler2D outputImage2;

void main() {
    vec4 color = unpackUnorm4x8(fragColor);
	outColor = color * vec4(1.0, 1.0, 0.0, 1.0);
}