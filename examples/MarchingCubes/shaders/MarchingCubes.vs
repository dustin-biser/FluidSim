// MarchingCubes.vs
#version 410

// texture-coordinate (u,v), lower-left corner of voxel.
layout (location = 0) in vec2 uvCoord; // In the range [0..1] per dimension.

// texture-coordinate layer
layout (location = 1) in float zLayer; // In the range [0..1].

uniform sampler3D densityTexture;


void main() {
	vec4 f0123; // Density values at all
	vec4 f4567; // ... 8 voxel corners.
	uint mc_case; // 0-255, edge table case.
	vec3 uvw; // minimum corner of voxel.

	uvw = vec3(uvCoord, zLayer);

	float densityValue = texture(densityTexture, uvw).r;

	gl_Position = vec4(densityValue, 0.0, 0.0, 1.0);


	// Convert from texture-space to [-1..1] range in world-space.
//	vec3 ws_Position = 2.0*vec3(uvCoord,zLayer)-vec3(1.0);

}