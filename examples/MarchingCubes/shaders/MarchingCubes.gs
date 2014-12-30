// MarchingCubes.gs
#version 410

layout(points) in;
layout(points, max_vertices = 1) out;

in vsOutputGsInput {
	vec3 wsPosition; // World-space position of minimum corner of voxel.
	vec3 uvw; // Minimum corner of voxel, normalized texture coordinate.
	vec4 f0123; // Density values at all
	vec4 f4567; // ... 8 voxel corners.
	uint mc_case; // 0-255, edge table case.
} gs_in;

void main() {
    gl_Position = wsPosition;
    EmitVertex();

    EndPrimitive();
}
