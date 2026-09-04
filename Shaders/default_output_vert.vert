#version 450
#pragma shader_stage(vertex)

layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 textureCoord;
layout (location = 0) out vec2 fragCoord;

layout(binding = 0) uniform CameraMatrix {
    mat4 transform;
} camera;

void main() {
    gl_Position = camera.transform * vec4(vertexPos, 1.0);
    fragCoord = textureCoord;
}