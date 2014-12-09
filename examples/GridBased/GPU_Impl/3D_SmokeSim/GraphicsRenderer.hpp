/**
* @brief GraphicsRenderer.hpp
*
* @author Dustin Biser
*/

#pragma once

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

//----------------------------------------------------------------------------------------
// Graphics Parameters
//----------------------------------------------------------------------------------------

// Vertex Attribute Indices
const int position_attribIndex = 0;

class GraphicsRenderer {
public:
    GraphicsRenderer(uint32 framebufferWidth, uint32 framebufferHeight, Camera * camera);

    ~GraphicsRenderer();

    void draw();
    void cleanup();

    void setupCamera();
    void setupShaders();
    void setupShaderUniforms();
    void setupVao();
    void setupBoundingCubeVertexData();
    void updateShaderUniforms();


private:
    Camera * camera;

    ShaderProgram shaderProgram_cubeEdges;

    GLuint boundingCube_vao;          // vertex array object
    GLuint boundingCube_vertexBuffer; // vertex buffer object (to store vertex position offsets in model-space)
    GLuint boundingCube_indexBuffer;  // element buffer object (indices)
};
