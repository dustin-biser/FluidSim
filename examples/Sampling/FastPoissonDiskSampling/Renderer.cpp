// Renderer.cpp

#include "Renderer.hpp"
#include "GlErrorCheck.hpp"

#include <glm/gtx/matrix_transform_2d.hpp>

using std::vector;

static const float32 PI = 3.14159265359f;
static const int32 vertex_position_attrib_index = 0;
static const int32 sample_position_attrib_index = 1;

//---------------------------------------------------------------------------------------
Renderer::Renderer(
		uint32 screenWidth,
		uint32 screenHeight,
		uint32 gridWidth,
		uint32 gridHeight,
		uint64 maxSamples
)
	: screenWidth(screenWidth),
	  screenHeight(screenHeight),
	  gridWidth(gridWidth),
	  gridHeight(gridHeight)
{
	configureOpenGLState();

	initShaderProgram();

	initGridVertices();
	initGridLineVertexAttribMapping();

	glGenVertexArrays(1, &vao_samples);
	initSampleVboStorage(maxSamples);
	setSamplePositionVertexAttribMapping();
	initCircleVboData();
	setCircleVertexAttribMapping();
}

//---------------------------------------------------------------------------------------
Renderer::~Renderer() {
	glDeleteBuffers(1, &vbo_gridLines);
	glDeleteVertexArrays(1, &vao_gridLines);
}

//---------------------------------------------------------------------------------------
void Renderer::configureOpenGLState() {
	// Render only the front face of geometry.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	// Setup depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0.0f, 1.0f);
	glEnable(GL_DEPTH_CLAMP);

	glClearDepth(1.0f);
	glClearColor(0.95, 0.95, 0.95, 0.0);
}

//---------------------------------------------------------------------------------------
void Renderer::initShaderProgram() {
	//-- shader_samples:
	{
		shader_samples.generateProgramObject();
		shader_samples.attachVertexShader("shaders/InstancedSamples.vs");
		shader_samples.attachFragmentShader("shaders/ScreenPrimative.fs");
		shader_samples.link();

		setSampleShaderUniforms();
	}

	//-- shader_grid:
	{
		shader_grid.generateProgramObject();
		shader_grid.attachVertexShader("shaders/ScreenPrimative.vs");
		shader_grid.attachFragmentShader("shaders/ScreenPrimative.fs");
		shader_grid.link();

		setGridShaderUniforms();
	}
}

//---------------------------------------------------------------------------------------
void Renderer::initGridVertices() {
	// Two vertices will be stored for each grid line.
	// Vertical grid lines span the full height of screen.
	// Horizontal grid lines span the full width of screen.
	// Grid indices start at bottom left of screen and increase to top right.

	const uint32 numVerticalLineVertices = 2*(gridWidth - 1);
	const uint32 numHorizontalLineVertices = 2*(gridHeight - 1);
	numGridVertices = numVerticalLineVertices + numHorizontalLineVertices;

	// Vertical grid line vertices are stored first in array then
	// Horizontal grid line vertices.
	//
	// gridVertices = [ vertical-line-vertices | horizontal-line-vertices ]
	//
	vec2 * gridVertices = new vec2[numGridVertices];

	// Allocate vertical grid line vertices
	float x_delta = 1.0f / float(gridWidth);
	for(uint32 i(0); i < numVerticalLineVertices; i += 2) {
		gridVertices[i] = vec2(x_delta * (i/2 + 1), 0.0f);
		gridVertices[i+1] = vec2(x_delta * (i/2 + 1), 1.0f);
	}

	// Allocate horizontal grid line vertices
	float y_delta = 1.0f / float(gridHeight);
	uint32 offset = numVerticalLineVertices;
	for(uint32 i(0); i < numHorizontalLineVertices; i += 2) {
		gridVertices[i+offset] = vec2(0.0f, y_delta * (i/2 + 1));
		gridVertices[i+1+offset] = vec2(1.0f, y_delta * (i/2 + 1));
	}

	glGenBuffers(1, &vbo_gridLines);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_gridLines);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*numGridVertices, gridVertices,
			GL_STATIC_DRAW);


	delete [] gridVertices;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void Renderer::initGridLineVertexAttribMapping() {
	glGenVertexArrays(1, &vao_gridLines);

	glBindVertexArray(vao_gridLines);
	glEnableVertexAttribArray(vertex_position_attrib_index);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_gridLines);
	glVertexAttribPointer(vertex_position_attrib_index, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void Renderer::initSampleVboStorage(
		uint64 maxSamples
) {
	//-- Allocate vbo storage for particle position data:
	glGenBuffers(1, &vbo_sample_positions);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sample_positions);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * maxSamples, (void *) NULL,
			GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void Renderer::initCircleVboData() {
	vector<vec2> circle_vertices;
	circle_vertices.push_back(vec2(0.0f, 0.0f));
	circle_vertices.push_back(vec2(1.0f, 0.0f));

	// Create equally spaced points on a unit sphere
	float x_prev, x = 1.0f;
	float y_prev, y = 0.0f;
	float dt = PI * 0.25f;
	float cos = std::cos(dt);
	float sin = std::sin(dt);

	for (int i(0); i < 7; ++i) {
		x_prev = x;
		y_prev = y;

		// Rotate next point by theta radians:
		x = (x_prev * cos) - (y_prev * sin);
		y = (x_prev * sin) + (y_prev * cos);

		circle_vertices.push_back(vec2(x, y));
	}

	circle_vertices.push_back(vec2(1.0f, 0.0f));

	glGenBuffers(1, &vbo_circle_mesh);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_circle_mesh);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * circle_vertices.size(),
			reinterpret_cast<const GLvoid *>(circle_vertices.data()), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void Renderer::setCircleVertexAttribMapping() {
	glBindVertexArray(vao_samples);
	glEnableVertexAttribArray(vertex_position_attrib_index);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_circle_mesh);
	glVertexAttribPointer(vertex_position_attrib_index, 2, GL_FLOAT,
			GL_FALSE, 0, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void Renderer::setSamplePositionVertexAttribMapping() {
	glBindVertexArray(vao_samples);
	glEnableVertexAttribArray(sample_position_attrib_index);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_sample_positions);
	glVertexAttribPointer(sample_position_attrib_index, 2, GL_FLOAT,
			GL_FALSE, 0, NULL);


	// Advance particle position attribute data once per instance.
	glVertexAttribDivisor(sample_position_attrib_index, 1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
glm::mat3 Renderer::getWorldScalingMatrix() {
	//-- OpenGL coordinate to world-space coordinate transform:
	// Scale by 2, then move origin to bottom right of screen.
	// Map (0,0) -> (-1,-1)
	// Map (1,1) -> (1,1)
	mat3 trans = glm::translate(mat3(), vec2(-1, -1));
	mat3 scale = glm::scale(mat3(), vec2(2.0f, 2.0f));

	return trans * scale;
}

//---------------------------------------------------------------------------------------
void Renderer::setSampleShaderUniforms() {
	mat3 scaleMatrix = glm::scale(mat3(), vec2(0.002,0.002));
	shader_samples.setUniform("modelMatrix", scaleMatrix);

	mat3 worldMatrix = getWorldScalingMatrix();
	shader_samples.setUniform("worldMatrix", worldMatrix);

	shader_samples.setUniform("primativeColor", vec3(0.45f, 0.45f, 0.88f));
}

//---------------------------------------------------------------------------------------
void Renderer::setSampleRadius(float r) {
	mat3 scaleMatrix = glm::scale(mat3(), vec2(r));
	shader_samples.setUniform("modelMatrix", scaleMatrix);
}

//---------------------------------------------------------------------------------------
void Renderer::setGridShaderUniforms() {
	mat3 modelMatrix = getWorldScalingMatrix();
	shader_grid.setUniform("modelMatrix", modelMatrix);

	shader_grid.setUniform("primativeColor", vec3(0.77f, 0.77f, 0.77f));
}

//---------------------------------------------------------------------------------------
void Renderer::uploadSamplePositions(
		const std::vector<glm::vec2> & samplePositions
) {
	glBindBuffer(GL_ARRAY_BUFFER, vbo_sample_positions);

	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(vec2) * samplePositions.size(),
			samplePositions.data());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void Renderer::renderGrid() {
	glBindVertexArray(vao_gridLines);

	shader_grid.enable();
	glDrawArrays(GL_LINES, 0, numGridVertices);
	shader_grid.disable();


	//-- Restore defaults:
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void Renderer::renderSamples(
		const std::vector<glm::vec2> & samplePositions
) {
	uploadSamplePositions(samplePositions);

	glBindVertexArray(vao_samples);

	shader_samples.enable();
	glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 10, GLsizei(samplePositions.size()));
	shader_samples.disable();


	//-- Restore defaults:
	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}
