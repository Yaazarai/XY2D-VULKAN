#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 fragCoord;
layout (location = 1) flat in uint fragColor;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D outputImage;

void main() {
    vec2 resolution = vec2(textureSize(outputImage, 0));
	vec2 pixelCoord = fragCoord * resolution;
	vec2 circlePos = resolution * 0.5;
	float circleRadius = 4.0;
	float dist = abs(length(pixelCoord - circlePos));
	
	outColor = vec4(0.0, 0.0, 0.0, 1.0);
	if (dist < circleRadius)
		outColor = vec4(1.0, 1.0, 1.0, 1.0);
}