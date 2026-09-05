#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 fragCoord;
layout(location = 0) out vec4 outColor;
layout(binding = 1) uniform sampler2D imageTexture;

void main() {
    //ivec2 texSize = imageSize(imageTexture);
    //ivec2 pixelCoord = ivec2(fragCoord * vec2(texSize));
    //outColor = imageLoad(imageTexture, pixelCoord);
	outColor = texture(imageTexture, fragCoord);
}