/**
* @brief SmokeGraphics.hpp
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
const int kScreenWidth = 800;
const int kScreenHeight = 800;
const int attribIndex_position = 0;
const int attribIndex_texCoord = 1;
const vec3 inkColor(0.01, 1.0, 0.2);

class SmokeGraphics {
public:
    void init(const Grid<float32> & inkGrid);
    void draw();
    void cleanup();

    void setupShaders();
    void setupUniforms();
    void setupVao();
    void setupBufferData();
    void uploadTextureData(const Grid<float32> & inkGrid);

    void createInkTexture(const Grid<float32> & inkGrid);

private:
    ShaderProgram shaderProgram;
    GLuint vao; // vertex array object
    GLuint vbo; // vertex buffer object
    GLuint ebo; // element buffer object (indices)

    GLuint tex2D_ink;
};
