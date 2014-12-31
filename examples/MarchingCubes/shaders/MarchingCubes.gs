// MarchingCubes.gs
#version 410

layout(points) in;
layout(points, max_vertices = 1) out;

in vsOutputGsInput {
	vec3 wsPosition; // World-space position of minimum corner of voxel.
	vec3 uvw; // Minimum corner of voxel, normalized texture coordinate.
	vec4 step; // voxel dimensions
	vec4 f0123; // Density values at all
	vec4 f4567; // ... 8 voxel corners.
	uint mc_case; // 0-255, edge table case.
} gs_in[1];

// Maps mc_case to sets of 3 cut edges.
uniform ivec3 triTable[256 * 5];

// Maps mc_case to number of triangles to output.
uniform uint case_to_numTriangles[256];

// Number of voxel edges.
const int numEdges = 12;

// Assume directed edges which start on a vertexA and end at a vertexB.

// Position within voxel of edge's vertexA, with vertex0 as origin.
uniform ivec3 edge_start[numEdges]; // .xyz in {0,1}

// Edge direction from vertexA to vertexB
uniform ivec3 edge_dir[numEdges]; // .xyz in {-1,0,1}


// Given an edge number, returns 1 at the vertex component location corresponding to
// edge's starting vertexA, 0 otherwise.
uniform vec4 cornerAmask0123[numEdges]; // .xyzw corresponds to vertex 0,1,2,3.
uniform vec4 cornerAmask4567[numEdges]; // .xyzw corresponds to vertex 4,5,6,7.

// Given an edge number, returns 1 at the vertex component location corresponding to
// edge's ending vertexB, 0 otherwise.
uniform vec4 cornerBmask0123[numEdges]; // .xyzw corresponds to vertex 0,1,2,3.
uniform vec4 cornerBmask4567[numEdges]; // .xyzw corresponds to vertex 4,5,6,7.


out float value;

void main() {
	// TODO Dustin - remove after testing:
		float dummy = float(triTable[0].x) +
		        case_to_numTriangles[0] +
		        numEdges +
		        edge_start[0].x +
		        edge_dir[0].x +
		        cornerAmask0123[0].x +
		        cornerAmask4567[0].x +
		        cornerBmask0123[0].x +
		        cornerBmask4567[0].x;

	value = 0;

    gl_Position = vec4(gs_in[0].wsPosition, 1.0);
    EmitVertex();

    EndPrimitive();
}
