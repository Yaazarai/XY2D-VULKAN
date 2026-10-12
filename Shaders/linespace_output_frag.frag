#version 450
#pragma shader_stage(fragment)

layout (location = 0) in vec2 fragCoord;
layout (location = 1) flat in uint fragColor;
layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D sceneImage;
layout(binding = 2) uniform sampler2D upperCascade;
layout(binding = 3) uniform sampler2D currentCascade;
layout(binding = 4) uniform cascadeInfo {
	int index;
	int count;
} cascade;

#define RADIANS(n) ((n) * 6.283185)
#define TAU 6.283185

void trace(vec2 rxy, vec2 dxy, float interval, inout vec4 radiance, inout vec4 transmit) {
	vec2 delta = dxy / max(abs(dxy.x), abs(dxy.y));
	float step_length = length(delta);
	vec2 inv_size = 1.0 / vec2(textureSize(sceneImage, 0));
	for(float ii = 0.0; ii < interval; ii ++) {
		vec2 ray = (rxy + (delta * ii)) * inv_size;
		if (clamp(ray.xy, 0.0, 1.0) != ray.xy) break;
		
		vec4 emiss = texture(sceneImage, ray);
		vec4 absrp = texture(sceneImage, ray);
		vec4 optic = -absrp * step_length;
		vec4 trans = exp2(optic);
		vec4 radnc = emiss * (1.0 - trans);
		radiance += radnc * transmit;
		transmit *= trans;
	}
}

void main() {
	vec2 world = vec2(textureSize(sceneImage, 0));
	vec2 extent = vec2(textureSize(currentCascade, 0));
	vec2 coord = fragCoord * world;
	outColor = vec4(0.0);
	
	for(float i = 0; i < 256.0; i++) {
		float theta = RADIANS((i / 256.0));
		vec2 delta = vec2(cos(theta), -sin(theta));
		
		vec4 radiance = vec4(0.0), transmit = vec4(1.0);
		trace(coord + delta * 0.25, delta, world.x, radiance, transmit);
		outColor += radiance;
	}
	
	outColor /= 256.0;
}

/*
	1. Get the nearest CN+1 probe, ray-circle intersect to its edge.
	2. Offset from edge by CN interval length.
	3. Get probe relative from sign(rayDelta)--nearest probe in direction of ray.
	4. Get ray-circle intersect to its edge.
	
	Using the ray circle intersect points p1 (near) and p2 (far) trace to and merge
	with p1 (near) then trace from p1 to p2 (far) and merge.
	
	Finally interpolate the merge results.
*/