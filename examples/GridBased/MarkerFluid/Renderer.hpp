// Renderer.hpp
#pragma once

#include <OpenGL/gl3.h>

#include <Synergy/Graphics/ShaderProgram.hpp>

#include "FluidSim/NumericTypes.hpp"

#include <vector>


class Renderer {
public:
	Renderer (
			uint32 screenWidth,
			uint32 screenHeight,
			uint32 gridWidth,
			uint32 gridHeight,
			uint64 maxSamples
	);

	~Renderer();

	void renderGrid();

	void renderSamples(
			const std::vector<glm::vec2> & samplePositions
	);

	void setSampleRadius(float r);

private:
	uint32 screenWidth;
	uint32 screenHeight;
	uint32 gridWidth;
	uint32 gridHeight;
	GLsizei numGridVertices;

	Synergy::ShaderProgram shader_samples;
	Synergy::ShaderProgram shader_grid;

	GLuint vao_gridLines;
	GLuint vbo_gridLines;

	GLuint vao_samples;
	GLuint vbo_sample_positions;
	GLuint vbo_circle_mesh;

	void configureOpenGLState();

	void initGridVertices();

	void initGridLineVertexAttribMapping();

	void initSampleVboStorage(
			uint64 maxSamples
	);

	void setSamplePositionVertexAttribMapping();

	void uploadSamplePositions(
			const std::vector<glm::vec2> & samplePositions
	);

	void initCircleVboData();

	void setCircleVertexAttribMapping();

	void initShaderProgram();

	glm::mat3 getWorldScalingMatrix();
	void setSampleShaderUniforms();
	void setGridShaderUniforms();
};
