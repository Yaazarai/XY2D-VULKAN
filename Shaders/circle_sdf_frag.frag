#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 fragCoord;
layout (location = 1) in vec4 fragColor;
layout(location = 0) out vec4 outColorA;
layout(location = 1) out vec4 outColorB;

layout(set = 3, binding = 0) uniform SceneData {
	vec2 renderSize;
	vec2 circlePosA;
	vec2 circlePosB;
	float circleRadius;
} scene;

void main() {
    vec2 pixelCoord = fragCoord * scene.renderSize;
	float distA = abs(length(pixelCoord - scene.circlePosA));
	float distB = abs(length(pixelCoord - scene.circlePosB));
	
	outColorA = vec4(1.0, 0.0, 0.0, 1.0);
	outColorB = vec4(1.0, 0.0, 0.0, 1.0);
	
	if (distA < scene.circleRadius) {
		outColorA = vec4(1.0, 1.0, 1.0, 1.0);
		outColorB = vec4(1.0, 1.0, 1.0, 1.0);
	}
	
	if (distB < scene.circleRadius) {
		outColorB = vec4(1.0, 1.0, 1.0, 1.0);
	}
}