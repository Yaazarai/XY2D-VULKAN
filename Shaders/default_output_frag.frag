#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 fragCoord;
layout (location = 1) flat in uint fragColor;
layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = unpackUnorm4x8(fragColor);
	outColor = color * vec4(1.0, 1.0, 0.0, 1.0);
}