// MarchingCubes.gs
#version 410

layout(points) in;

layout(points, max_vertices = 15) out;

in vsOutputGsInput {
	vec3 wsPosition; // World-space position of voxel's minimum corner.
	vec3 wsVoxelSize; // World-space voxel size in each dimension xyz.
	vec4 f0123; // Density values at all
	vec4 f4567; // ... 8 voxel corners.
	uint mc_case; // 0-255, triTable case.
	float isoValue; // iso-surface value.
} gs_in[1];


// Number of edges per voxel.
const int numEdges = 12;

// Total number of marching cube cases.
const int numCases = 256;



// Maps mc_case to a set of 5 edge number triplets (RGB) that the isosurface intersects.
// If a set starts with -1, then there are no more intersecting edges for the case.
uniform isampler2D triTable;

// Maps mc_case to number of triangles to output.  Range [0..5]
uniform uint case_to_numTriangles[numCases];

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

layout (stream = 0) out vec3 outWsPosition;
layout (stream = 1) out vec3 outWsNormal;

void placeVertOnEdge(in int edgeNum, out vec3 vertexPosition) {

	// Obtain value at vertex A.
	float aValue = dot(cornerAmask0123[edgeNum], gs_in[0].f0123) +
	               dot(cornerAmask4567[edgeNum], gs_in[0].f4567);

	// Obtain value at vertex B.
	float bValue = dot(cornerBmask0123[edgeNum], gs_in[0].f0123) +
	               dot(cornerBmask4567[edgeNum], gs_in[0].f4567);

	// Distance along edge for the isosurface crossing.
	float t = (gs_in[0].isoValue - aValue) / (bValue - aValue); // [0..1]

	vec3 pos_within_cell = edge_start[edgeNum] + t * edge_dir[edgeNum];

	vertexPosition = gs_in[0].wsPosition + pos_within_cell * gs_in[0].wsVoxelSize;

	outWsPosition = vertexPosition;

	EmitStreamVertex(0);
	EndStreamPrimitive(0);
}

void computeNormal(vec3 vertexA, vec3 vertexB, vec3 vertexC) {
	vec3 dir_a_to_b = vertexB - vertexA;
	vec3 dir_a_to_c = vertexC - vertexA;
	vec3 normal = normalize(cross(dir_a_to_b, dir_a_to_c));

	// Repeat 3 times, so each vertex position has a corresponding normal.
	for(int i = 0; i < 3; ++i) {
		// Must reassign output variables after each EmitStreamVertex() call.
		outWsNormal = normal;
		EmitStreamVertex(1);
		EndStreamPrimitive(1);
	}
}

void main() {

	uint mc_case = gs_in[0].mc_case;
	uint numTriangles = case_to_numTriangles[mc_case];

	for(int i = 0; i < numTriangles; ++i) {
		ivec3 edges = texelFetch(triTable, ivec2(i, mc_case), 0).rgb;

		vec3 vertexPosition[3];

		placeVertOnEdge(edges[0], vertexPosition[0]);

		placeVertOnEdge(edges[1], vertexPosition[1]);

		placeVertOnEdge(edges[2], vertexPosition[2]);

		computeNormal(vertexPosition[0], vertexPosition[1], vertexPosition[2]);
	}
}
