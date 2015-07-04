/**
* @brief SmokeGraphics.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/Grid.hpp"
using namespace FluidSim;

#include <Synergy/Synergy.hpp>
using namespace Synergy;

enum class CellType : bool;

//----------------------------------------------------------------------------------------
// Graphics Parameters
//----------------------------------------------------------------------------------------
const int kScreenWidth = 800;
const int kScreenHeight = 800;

// Vertex Attribute Indices
const int position_attribIndex = 0;
const int texCoord_attribIndex = 1;
const int centerPosition_attribIndex = 2;

const vec3 inkColor(0.5, 0.45, 1.0);
const vec3 solidCellColor(0.8, 0.2, 0.2);

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
    void uploadSolidCellData(const Grid<CellType> & cellGrid);

    void createInkTexture(const Grid<float32> & inkGrid);

private:

    ShaderProgram screenQuad_shaderProgram;
    ShaderProgram solidCell_shaderProgram;

    // Reuse vertex data for both screen quad, and solid cells:
    GLuint vao; // vertex array object
    GLuint vbo; // vertex buffer object (to store vertex position offsets in modelspace)
    GLuint vbo_solidCellCenterPositions;
    GLuint ebo; // element buffer object (indices)

    GLuint tex2D_ink;

    uint32 num_solid_cells;
};
