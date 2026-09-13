#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec2 vertexPos;
layout (location = 1) in vec2 textureCoord;
layout (location = 2) in uint vertexColor;

layout (location = 0) out vec2 fragCoord;
layout (location = 1) flat out uint fragColor;

layout(binding = 0) uniform CameraMatrix {
	mat4 projection;
	mat2 rotation;
	vec2 position;
	vec2 scale;
	vec2 origin;
} camera;

void main() {
    vec2 offset = vertexPos.xy - camera.origin;
    offset = camera.rotation * offset;
    vec2 vertex = offset + camera.origin + camera.position;
    gl_Position = camera.projection * vec4(vertex, 0.0, 1.0);
    fragCoord = textureCoord;
	fragColor = vertexColor;
}