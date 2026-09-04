#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 fragCoord;
layout (location = 1) in vec4 fragColor;
layout(location = 0) out vec4 outColor;
layout(binding = 0, r11f_g11f_b10f) uniform readonly image2D imageTexture;

void main() {
    ivec2 texSize = imageSize(imageTexture);
    ivec2 pixelCoord = ivec2(fragCoord * vec2(texSize));
    outColor = imageLoad(imageTexture, pixelCoord);
}