/**
* @brief SmokeGraphics3D.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/Grid.hpp"
using namespace FluidSim;

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

//----------------------------------------------------------------------------------------
// Graphics Parameters
//----------------------------------------------------------------------------------------
const int kScreenWidth = 1024;
const int kScreenHeight = 768;

// Vertex Attribute Indices
const int position_attribIndex = 0;

class SmokeGraphics3D {
public:
    void init(Camera * camera);
    void draw();
    void cleanup();

    void setupCamera();
    void setupShaders();
    void setupUniforms();
    void setupVao();
    void setupBufferData();


private:
    Camera * camera;

    ShaderProgram shaderProgram;

    // Reuse vertex data for both screen quad, and solid cells:
    GLuint vao; // vertex array object
    GLuint vbo; // vertex buffer object (to store vertex position offsets in model-space)
    GLuint ebo; // element buffer object (indices)
};
