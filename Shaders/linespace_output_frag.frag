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

#define PI 3.141592653589793

void trace(vec2 probe, vec2 merge, vec2 extent, inout vec4 radiance, inout vec4 transmit) {
	vec4 ray = vec4(probe, merge / max(abs(merge.y), abs(merge.x))) / extent.xyxy;
	for(float ii = 0.0, op = length(ray.zw * extent.xy); ii < worldExt.x; ii ++, ray.xy += ray.zw) {
		if (clamp(ray.xy, 0.0, 1.0) != ray.xy) break;
		vec3 tt = sqrt(exp2(-pow(texture(absorption, ray.xy).rgb * op, 2.0)));
		vec3 rr = texture(emissivity, ray.xy).rgb;
		radiance.rgb += (rr * (1.0 - tt)) * transmit.rgb;
		transmit.rgb *= tt;
	}
}

void main() {
	
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