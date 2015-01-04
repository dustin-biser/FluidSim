#include "MarchingCubesRenderer.hpp"
#include "MarchingCubesExample.hpp"

using namespace Rigid3D;


//---------------------------------------------------------------------------------------
MarchingCubesRenderer::MarchingCubesRenderer(
    uint32 gridWidth,
    uint32 gridHeight,
    uint32 gridDepth)
    : gridWidth(gridWidth),
      gridHeight(gridHeight),
      gridDepth(gridDepth),
      numVoxelsPerLayer(0),
	  transformFeedbackBufferSize(0)
{
    setupShaders();
	generateTriTableTexture();
    setShaderUniforms();

    setupVoxelUvCoordVboData();
    setupVoxelZLayerVboData();
	setupVaoForVoxelData();

    setupSamplerObject();

	setupTransformFeedback();
	setupVaoForIsoSurfaceTriangles();

	setupVoxelEdgesVertexBuffer();
	setupVoxelEdgesVao();
}

//---------------------------------------------------------------------------------------
MarchingCubesRenderer::~MarchingCubesRenderer() {

}

//---------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupShaders() {

	//-- shaderProgram_genIsoSurface:
	{
		shaderProgram_genIsoSurface.generateProgramObject();
		shaderProgram_genIsoSurface.attachVertexShader
				("examples/MarchingCubes/shaders/MarchingCubes.vs");

		shaderProgram_genIsoSurface.attachGeometryShader
				("examples/MarchingCubes/shaders/MarchingCubes.gs");

		// Link outWsPosition to stream index 0.
		// Link outWsNormal to stream index 1.
		const GLchar *feedbackVaryings[] = {"outWsPosition", "outWsNormal"};
		glTransformFeedbackVaryings(shaderProgram_genIsoSurface.getProgramObject(), 2,
				feedbackVaryings, GL_SEPARATE_ATTRIBS);

		shaderProgram_genIsoSurface.link();
	}


	//-- shaderProgram_renderIsoSurface:
	{
		shaderProgram_renderIsoSurface.generateProgramObject();
		shaderProgram_renderIsoSurface.attachVertexShader
				("examples/MarchingCubes/shaders/RenderIsoSurface.vs");
		shaderProgram_renderIsoSurface.attachFragmentShader
				("examples/MarchingCubes/shaders/RenderIsoSurface.fs");
		shaderProgram_renderIsoSurface.link();
	}


	//-- shaderProgram_voxelEdges:
	{
		shaderProgram_voxelEdges.generateProgramObject();
		shaderProgram_voxelEdges.attachVertexShader
				("examples/MarchingCubes/shaders/LineRender.vs");
		shaderProgram_voxelEdges.attachFragmentShader
				("examples/MarchingCubes/shaders/LineRender.fs");
		shaderProgram_voxelEdges.link();
	}
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setShaderUniforms() {

    shaderProgram_genIsoSurface.setUniform("volumeData", volumeData_texUnitOffset);
	shaderProgram_genIsoSurface.setUniform("triTable", triTable_texUnitOffset);
    shaderProgram_genIsoSurface.setUniform("gridWidth", gridWidth);
    shaderProgram_genIsoSurface.setUniform("gridHeight", gridHeight);
    shaderProgram_genIsoSurface.setUniform("gridDepth", gridDepth);
	uploadUniformArrays();

	shaderProgram_renderIsoSurface.setUniform("color", vec3(0.1f,0.2f,0.8f));

	shaderProgram_voxelEdges.setUniform("lineColor", vec3(0.7f, 0.7f, 0.7f));
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::uploadUniformArrays() {
	uint32 case_to_numTriangles[256] = {
			0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3,
			3, 4, 3, 4, 4, 3, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 2,
			3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4,
			4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 2, 3, 3, 4, 3, 4, 2, 3,
			3, 4, 4, 5, 4, 5, 3, 2, 3, 4, 4, 3, 4, 5, 3, 2, 4, 5, 5, 4, 5, 2, 4, 1, 1, 2,
			2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3,
			4, 3, 5, 2, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 3, 4, 4, 3, 4, 5,
			5, 4, 4, 3, 5, 2, 5, 4, 2, 1, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2,
			3, 4, 4, 5, 4, 5, 5, 2, 4, 3, 5, 4, 3, 2, 4, 1, 3, 4, 4, 5, 4, 5, 3, 4, 4, 5,
			5, 2, 3, 4, 2, 1, 2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0
	};

	GLint case_to_numTriangles_location =
			shaderProgram_genIsoSurface.getUniformLocation("case_to_numTriangles");
	glUseProgram(shaderProgram_genIsoSurface.getProgramObject());
	glUniform1uiv(case_to_numTriangles_location, 256, case_to_numTriangles);
	CHECK_GL_ERRORS;



	// Position within voxel of edge's starting vertexA, with vertex0 as origin.
	int edge_start[12 * 3] = {
			0, 0, 0,
			1, 0, 0,
			1, 1, 0,
			0, 0, 0,
			0, 0, 1,
			1, 0, 1,
			1, 1, 1,
			0, 1, 1,
			0, 0, 0,
			1, 0, 0,
			1, 1, 0,
			0, 1, 0
	};

	GLint edge_start_location = shaderProgram_genIsoSurface.getUniformLocation("edge_start");
	glUseProgram(shaderProgram_genIsoSurface.getProgramObject());
	glUniform3iv(edge_start_location, 12, edge_start);
	CHECK_GL_ERRORS;



	// Edge direction from vertexA to vertexB
	int edge_dir[12 * 3] = {
			1, 0, 0,
			0, 1, 0,
			-1, 0, 0,
			0, 1, 0, 
			1, 0, 0,
			0, 1, 0,
			-1, 0, 0,
			0, 1, 0,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1,
			0, 0, 1
	};

	GLint edge_dir_location = shaderProgram_genIsoSurface.getUniformLocation("edge_dir");
	glUseProgram(shaderProgram_genIsoSurface.getProgramObject());
	glUniform3iv(edge_dir_location, 12, edge_dir);
	CHECK_GL_ERRORS;

	
	
	// Given an edge number, returns 1 at the vertex component location corresponding to
	// edge's starting vertexA.
	// .xyzw corresponds to vertex 0,1,2,3.
	float32 cornerAmask0123[12 * 4] = {
			1, 0, 0, 0,     // edge 0, starts at vertex 0
			0, 1, 0, 0,     // edge 1, starts at vertex 1
			0, 0, 1, 0,     // edge 2, starts at vertex 2
			1, 0, 0, 0,     // edge 3, starts at vertex 0

			0, 0, 0, 0,     // edge 4, starts at vertex 4
			0, 0, 0, 0,     // edge 5, starts at vertex 5
			0, 0, 0, 0,     // edge 6, starts at vertex 6
			0, 0, 0, 0,     // edge 7, starts at vertex 4

			1, 0, 0, 0,     // edge 8, starts at vertex 0
			0, 1, 0, 0,     // edge 9, starts at vertex 1
			0, 0, 1, 0,     // edge 10, starts at vertex 2
			0, 0, 0, 1,     // edge 11, starts at vertex 3
	};
	
	GLint cornerAmask0123_location = shaderProgram_genIsoSurface.getUniformLocation("cornerAmask0123");
	glUseProgram(shaderProgram_genIsoSurface.getProgramObject());
	glUniform4fv(cornerAmask0123_location, 12, cornerAmask0123);
	CHECK_GL_ERRORS;


	// Given an edge number, returns 1 at the vertex component location corresponding to
	// edge's starting vertexA.
	// .xyzw corresponds to vertex 4,5,6,7.
	float32 cornerAmask4567[12 * 4] = {
			0, 0, 0, 0,     // edge 0, starts at vertex 0
			0, 0, 0, 0,     // edge 1, starts at vertex 1
			0, 0, 0, 0,     // edge 2, starts at vertex 2
			0, 0, 0, 0,     // edge 3, starts at vertex 3

			1, 0, 0, 0,     // edge 4, starts at vertex 4
			0, 1, 0, 0,     // edge 5, starts at vertex 5
			0, 0, 1, 0,     // edge 6, starts at vertex 6
			1, 0, 0, 0,     // edge 7, starts at vertex 4

			0, 0, 0, 0,     // edge 8, starts at vertex 0
			0, 0, 0, 0,     // edge 9, starts at vertex 1
			0, 0, 0, 0,     // edge 10, starts at vertex 2
			0, 0, 0, 0,     // edge 11, starts at vertex 3
	};

	GLint cornerAmask4567_location = shaderProgram_genIsoSurface.getUniformLocation("cornerAmask4567");
	glUseProgram(shaderProgram_genIsoSurface.getProgramObject());
	glUniform4fv(cornerAmask4567_location, 12, cornerAmask4567);
	CHECK_GL_ERRORS;



	// Given an edge number, returns 1 at the vertex component location corresponding to
	// edge's ending vertexB.
	// .xyzw corresponds to vertex 0,1,2,3.
	float32 cornerBmask0123[12 * 4] = {
			0, 1, 0, 0,     // edge 0, ends at vertex 1
			0, 0, 1, 0,     // edge 1, ends at vertex 2
			0, 0, 0, 1,     // edge 2, ends at vertex 3
			0, 0, 0, 1,     // edge 3, ends at vertex 3

			0, 0, 0, 0,     // edge 4, ends at vertex 5
			0, 0, 0, 0,     // edge 5, ends at vertex 6
			0, 0, 0, 0,     // edge 6, ends at vertex 7
			0, 0, 0, 0,     // edge 7, ends at vertex 7

			0, 0, 0, 0,     // edge 8, ends at vertex 4
			0, 0, 0, 0,     // edge 9, ends at vertex 5
			0, 0, 0, 0,     // edge 10, ends at vertex 6
			0, 0, 0, 0,     // edge 11, ends at vertex 7
	};

	GLint cornerBmask0123_location = shaderProgram_genIsoSurface.getUniformLocation("cornerBmask0123");
	glUseProgram(shaderProgram_genIsoSurface.getProgramObject());
	glUniform4fv(cornerBmask0123_location, 12, cornerBmask0123);
	CHECK_GL_ERRORS;



	// Given an edge number, returns 1 at the vertex component location corresponding to
	// edge's ending vertexB.
	// .xyzw corresponds to vertex 4,5,6,7.
	float32 cornerBmask4567[12 * 4] = {
			0, 0, 0, 0,     // edge 0, ends at vertex 1
			0, 0, 0, 0,     // edge 1, ends at vertex 2
			0, 0, 0, 0,     // edge 2, ends at vertex 3
			0, 0, 0, 0,     // edge 3, ends at vertex 3

			0, 1, 0, 0,     // edge 4, ends at vertex 5
			0, 0, 1, 0,     // edge 5, ends at vertex 6
			0, 0, 0, 1,     // edge 6, ends at vertex 7
			0, 0, 0, 1,     // edge 7, ends at vertex 7

			1, 0, 0, 0,     // edge 8, ends at vertex 4
			0, 1, 0, 0,     // edge 9, ends at vertex 5
			0, 0, 1, 0,     // edge 10, ends at vertex 6
			0, 0, 0, 1,     // edge 11, ends at vertex 7
	};

	GLint cornerBmask4567_location = shaderProgram_genIsoSurface.getUniformLocation("cornerBmask4567");
	glUseProgram(shaderProgram_genIsoSurface.getProgramObject());
	glUniform4fv(cornerBmask4567_location, 12, cornerBmask4567);
	CHECK_GL_ERRORS;



    glUseProgram(0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::generateTriTableTexture() {
	// Maps mc_case to sets of 3 cut edges.
	int triTable[256 * 15] = {
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1,
			3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1,
			3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1,
			3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1,
			9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1,
			1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1,
			9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1,
			2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1,
			8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1,
			9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1,
			4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1,
			3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1,
			1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1,
			4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1,
			4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1,
			9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1,
			1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1,
			5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1,
			2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1,
			9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1,
			0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1,
			2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1,
			10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1,
			4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1,
			5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1,
			5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1,
			9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1,
			0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1,
			1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1,
			10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1,
			8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1,
			2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1,
			7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1,
			9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1,
			2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1,
			11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1,
			9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1,
			5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0,
			11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0,
			11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1,
			1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1,
			9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1,
			5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1,
			2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1,
			0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1,
			5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1,
			6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1,
			0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1,
			3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1,
			6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1,
			5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1,
			1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1,
			10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1,
			6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1,
			1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1,
			8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1,
			7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9,
			3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1,
			5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1,
			0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1,
			9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6,
			8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1,
			5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11,
			0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7,
			6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1,
			10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1,
			10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1,
			8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1,
			1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1,
			3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1,
			0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1,
			10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1,
			0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1,
			3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1,
			6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1,
			9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1,
			8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1,
			3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1,
			6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1,
			0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1,
			10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1,
			10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1,
			1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1,
			2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9,
			7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1,
			7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1,
			2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7,
			1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11,
			11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1,
			8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6,
			0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1,
			7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1,
			10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1,
			2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1,
			6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1,
			7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1,
			2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1,
			1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1,
			10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1,
			10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1,
			0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1,
			7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1,
			6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1,
			8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1,
			9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1,
			6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1,
			1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1,
			4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1,
			10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3,
			8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1,
			0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1,
			1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1,
			8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1,
			10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1,
			4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3,
			10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1,
			5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1,
			11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1,
			9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1,
			6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1,
			7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1,
			3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6,
			7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1,
			9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1,
			3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1,
			6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8,
			9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1,
			1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4,
			4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10,
			7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1,
			6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1,
			3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1,
			0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1,
			6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1,
			1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1,
			0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10,
			11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5,
			6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1,
			5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1,
			9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1,
			1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8,
			1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6,
			10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1,
			0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1,
			5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1,
			10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1,
			11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1,
			0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1,
			9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1,
			7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2,
			2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1,
			8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1,
			9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1,
			9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2,
			1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1,
			9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1,
			9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1,
			5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1,
			0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1,
			10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4,
			2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1,
			0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11,
			0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5,
			9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1,
			5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1,
			3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9,
			5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1,
			8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1,
			0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1,
			9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1,
			0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1,
			1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1,
			3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4,
			4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1,
			9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3,
			11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1,
			11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1,
			2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1,
			9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7,
			3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10,
			1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1,
			4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1,
			4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1,
			0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1,
			3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1,
			3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1,
			0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1,
			9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1,
			1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
			-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
	};

	glGenTextures(1, &triTable_texture2d);
	glBindTexture(GL_TEXTURE_2D, triTable_texture2d);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8I, 5 /*width*/, 256 /*height*/,
			0, GL_RGB_INTEGER, GL_INT, triTable);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// border color defaults to 0.

	glBindTexture(GL_TEXTURE_2D, 0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupVoxelEdgesVao() {
	glGenVertexArrays(1, &vao_voxelEdgeLines);
	glBindVertexArray(vao_voxelEdgeLines);

	// Set index buffer binding for VAO.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxelEdges_indexBuffer);

	// Map position buffer data into vertex attribute index.
	glBindBuffer(GL_ARRAY_BUFFER, voxelEdges_vertexBuffer);
	glEnableVertexAttribArray(position_attrib_index);
	glVertexAttribPointer(position_attrib_index, 3, GL_FLOAT, GL_FALSE, 0,
			reinterpret_cast<void *>(0));


	//-- Restore defaults:
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupVoxelEdgesVertexBuffer() {
	//  Voxel vertex offsets in model-space
	float32 voxelVertices[] = {
			-0.5f, -0.5f, -0.5f,    // 0 Left Bottom Back
			 0.5f, -0.5f, -0.5f,    // 1 Right Bottom Back
			-0.5f, -0.5f,  0.5f,    // 2 Left Bottom Front
			 0.5f, -0.5f,  0.5f,    // 3 Right Bottom Front
			-0.5f,  0.5f, -0.5f,    // 4 Left Top Back
		     0.5f,  0.5f, -0.5f,    // 5 Right Top Back
			-0.5f,  0.5f,  0.5f,    // 6 Left Top Front
			 0.5f,  0.5f,  0.5f,    // 7 Right Top Front
	};
	// Upload Vertex Position Data:
	glGenBuffers(1, &voxelEdges_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, voxelEdges_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(voxelVertices), voxelVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS;


	// Voxel corner indices for constructing GL_LINES.
	// Every two indices is a new line.
	GLushort indices [] = {
			2,3,3,1,1,0,0,2,   // Bottom Face
			6,7,7,5,5,4,4,6,   // Top Face
			2,6,    // Left Front Vertical
			3,7,    // Right Front Vertical
			0,4,    // Left Back Vertical
			1,5,    // Right Back Vertical

	};
	// Upload Index Data:
	glGenBuffers(1, &voxelEdges_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, voxelEdges_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupVoxelUvCoordVboData() {
    glGenBuffers(1, &vbo_voxelUvCoords);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_voxelUvCoords);

    // Every 2 floats will represent the texture-space coordinate of lower-left corner
    // vertex of voxel cell.
    vector<vec2> texelCoords;

    vec2 uv(0.0f);

    // i spans [0..gridWidth-2]
    // j spans [0..gridHeight-2]
    for (int32 j = 0; j <= gridHeight-2; ++j) {
		for (int32 i = 0; i <= gridWidth-2; ++i) {

            // texel center coordinates
            uv.x = (i + 0.5f) / gridWidth;
            uv.y = (j + 0.5f) / gridHeight;

            texelCoords.push_back(uv);
            ++numVoxelsPerLayer;
        }
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*texelCoords.size(), texelCoords.data(),
            GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupVoxelZLayerVboData() {
    glGenBuffers(1, &vbo_voxelZLayer);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_voxelZLayer);

    vector<float32> layerCoords;

    float32 zLayer;

    // i spans [0..gridDepth-2]
    for (int32 i = 0; i <= gridDepth-2; ++i) {

        // texel center z position
        zLayer = (i + 0.5) / gridDepth;

        layerCoords.push_back(zLayer);
    }
    glBufferData(GL_ARRAY_BUFFER, sizeof(float32) * layerCoords.size(),
            layerCoords.data(), GL_STATIC_DRAW);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Set Vao Data Bindings
void MarchingCubesRenderer::setupVaoForVoxelData() {
    glGenVertexArrays(1, &vao_voxelData);
    glBindVertexArray(vao_voxelData);
    glEnableVertexAttribArray(uvCoord_attrib_index);
    glEnableVertexAttribArray(zLayerCoord_attrib_index);

    //-- voxel uv coordinates:
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_voxelUvCoords);

        int32 elementsPerVertex = 2;
        int32 stride = 0;
        int32 offsetToFirstElement = 0;
        glVertexAttribPointer(uvCoord_attrib_index, elementsPerVertex, GL_FLOAT,
                GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    //-- voxel z layer texture coordinates (instanced):
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_voxelZLayer);

        int32 elementsPerVertex = 1;
        int32 stride = 0;
        int32 offsetToFirstElement = 0;
        glVertexAttribPointer(zLayerCoord_attrib_index, elementsPerVertex, GL_FLOAT,
                GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

        // Advance once per 2D layer of GL_POINT primitives
        glVertexAttribDivisor(zLayerCoord_attrib_index, 1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    //-- Restore Defaults:
    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupVaoForIsoSurfaceTriangles() {
	glGenVertexArrays(1, &vao_isoSurfaceTriangles);
	glBindVertexArray(vao_isoSurfaceTriangles);
	glEnableVertexAttribArray(position_attrib_index);
	glEnableVertexAttribArray(normal_attrib_index);

	// Map transform feedback stream buffers to vertex attribute locations for rendering
	// isosurface triangles.
	{
		glBindBuffer(GL_ARRAY_BUFFER, streamBuffer_wsPositions);
		glVertexAttribPointer(position_attrib_index, 3, GL_FLOAT,
				GL_FALSE, 0, reinterpret_cast<void *>(0));

		glBindBuffer(GL_ARRAY_BUFFER, streamBuffer_wsNormals);
		glVertexAttribPointer(normal_attrib_index, 3, GL_FLOAT,
				GL_FALSE, 0, reinterpret_cast<void *>(0));
	}


	//-- Restore Defaults:
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupSamplerObject() {
    glGenSamplers(1, &sampler_volumeData);
    glSamplerParameterf(sampler_volumeData, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameterf(sampler_volumeData, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::setupTransformFeedback() {

	//-- Set an upper limit for vertex buffer transform feedback storage:
	int vec3_size = sizeof(GLfloat) * 3;
	int vertices_per_triangle = 3;
	int one_mega_byte = sizeof(char) * 1000000;
	transformFeedbackBufferSize = vertices_per_triangle * vec3_size;
	transformFeedbackBufferSize *=
			std::min(std::max(64, int(gridWidth * gridHeight * gridDepth)),  one_mega_byte);


	//-- Allocate stream buffer storage for vertex positions:
	{
		glGenBuffers(1, &streamBuffer_wsPositions);
		glBindBuffer(GL_ARRAY_BUFFER, streamBuffer_wsPositions);

		glBufferData(GL_ARRAY_BUFFER, transformFeedbackBufferSize, nullptr, GL_STREAM_COPY);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	//-- Allocate stream buffer storage for vertex normals:
	{
		glGenBuffers(1, &streamBuffer_wsNormals);
		glBindBuffer(GL_ARRAY_BUFFER, streamBuffer_wsNormals);

		glBufferData(GL_ARRAY_BUFFER, transformFeedbackBufferSize, nullptr, GL_STREAM_COPY);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}


	//-- Setup transform feedback stream index binding points:
	{
		glGenTransformFeedbacks(1, &transformFeedbackObj);
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbackObj);

		// vertex positions:
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,
				streamIndex_wsPositions, streamBuffer_wsPositions);

		// vertex normals:
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER,
				streamIndex_wsNormals, streamBuffer_wsNormals);

		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::inspectTransformFeedbackBuffer() {

	// Empty out GL pipeline so that stream buffers have data.
	glFlush();
	glFinish();

    GLsizei numElements =  transformFeedbackBufferSize / sizeof(GLfloat);
    GLfloat * stream0Data = new GLfloat[numElements];
	GLfloat * stream1Data = new GLfloat[numElements];

	glBindBuffer(GL_ARRAY_BUFFER, streamBuffer_wsPositions);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, transformFeedbackBufferSize, stream0Data);

	glBindBuffer(GL_ARRAY_BUFFER, streamBuffer_wsNormals);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, transformFeedbackBufferSize, stream1Data);


	delete [] stream0Data;
	delete [] stream1Data;

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::updateShaderUniforms(const Rigid3D::Camera &camera){
	mat4 projMatrix = camera.getProjectionMatrix();
	mat4 viewMatrix = camera.getViewMatrix();
	mat4 mvp_Matrix = projMatrix * viewMatrix;

	shaderProgram_renderIsoSurface.setUniform("MVP_Matrix", mvp_Matrix);
	shaderProgram_renderIsoSurface.setUniform("NormalMatrix", glm::transpose(glm::inverse(viewMatrix)));

	shaderProgram_voxelEdges.setUniform("MVP_Matrix", mvp_Matrix);
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::render(
        const Rigid3D::Camera & camera,
        GLuint volumeData_texture3d,
        float32 isoSurfaceThreshold
){
	generateIsoSurfaceTriangles(volumeData_texture3d, isoSurfaceThreshold);
	renderIsoSurface(camera);
	renderVoxelEdges(camera);
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::generateIsoSurfaceTriangles(
	GLuint volumeData_texture3d,
	float isoSurfaceThreshold
){
	shaderProgram_genIsoSurface.setUniform("isoSurfaceThreshold", isoSurfaceThreshold);

	// Prevent rasterization.
	glEnable(GL_RASTERIZER_DISCARD);

	glBindVertexArray(vao_voxelData);

	glActiveTexture(GL_TEXTURE0 + volumeData_texUnitOffset);
	glBindTexture(GL_TEXTURE_3D, volumeData_texture3d);
	glBindSampler(volumeData_texUnitOffset, sampler_volumeData);

	glActiveTexture(GL_TEXTURE0 + triTable_texUnitOffset);
	glBindTexture(GL_TEXTURE_2D, triTable_texture2d);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbackObj);

	shaderProgram_genIsoSurface.enable();
		glBeginTransformFeedback(GL_POINTS);
//		glDrawArraysInstanced(GL_POINTS, 0, numVoxelsPerLayer, gridDepth - 1);

		// TODO Dustin - remove:
			// Process only first voxel:
			glDrawArrays(GL_POINTS, 0, 1);

		glEndTransformFeedback();
	shaderProgram_genIsoSurface.disable();

	inspectTransformFeedbackBuffer();

	//-- Restore defaults:
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_3D, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindSampler(volumeData_texUnitOffset, 0);
	glDisable(GL_RASTERIZER_DISCARD);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::renderIsoSurface(const Rigid3D::Camera & camera) {
	updateShaderUniforms(camera);

	glBindVertexArray(vao_isoSurfaceTriangles);

	// Contains transform feedback primitive count needed for draw call.
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedbackObj);

	shaderProgram_renderIsoSurface.enable();
		glDrawTransformFeedback(GL_TRIANGLES, transformFeedbackObj);
	shaderProgram_renderIsoSurface.disable();

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void MarchingCubesRenderer::renderVoxelEdges(const Rigid3D::Camera &camera) {
	updateShaderUniforms(camera);

	glBindVertexArray(vao_voxelEdgeLines);

	shaderProgram_voxelEdges.enable();
	glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);
	shaderProgram_voxelEdges.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

