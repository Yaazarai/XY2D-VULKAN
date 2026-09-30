#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 fragCoord;
layout (location = 1) flat in uint fragColor;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D upperCascade;
layout(binding = 2) uniform cascadeInfo {
	int index;
	int count;
} cascade;

void main() {
	ivec2 extent = textureSize(upperCascade, 0);
	ivec2 xypos = ivec2(fragCoord * extent);
	int interval = 1 << (cascade.index * 2);
	ivec2 probe = xypos % interval;
	
	int probeIndex = probe.x;
	int rayIndex = probe.y;
	
	float pind = float(probeIndex);
	float rind = float(rayIndex);
	outColor = vec4(vec2(pind, rind) / interval, 0.0, 1.0);
}

/*
	2x/4x Ring Based HRC using 4x pre-averaging.
*/