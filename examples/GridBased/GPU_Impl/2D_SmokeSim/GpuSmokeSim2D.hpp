/**
* @brief GpuSmokeSim2D.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/NumericTypes.hpp"
using namespace FluidSim;

#include "Utils/GlfwOpenGlWindow.hpp"

#include <Rigid3D/Graphics/ShaderProgram.hpp>
using namespace Rigid3D;

//----------------------------------------------------------------------------------------
// Simulation Parameters
//----------------------------------------------------------------------------------------
const float32 kSecondsPerFrame = 1.0f;
const float32 kDt = 0.008;
const int32 solver_iterations = 60;
const int32 kScreenWidth = 1024;
const int32 kScreenHeight = 768;

//----------------------------------------------------------------------------------------
// Fluid Parameters
//----------------------------------------------------------------------------------------
const float32 temp_0 = 273.0f; // Ambient Temperature in Kelvin
const float32 kBuoyant_d = 0.8f;// Density coefficient for buoyant force.
const float32 kBuoyant_t = 0.34f; // Temperature coefficient for buoyant force.
const float32 kDensity = 1.0f;

//----------------------------------------------------------------------------------------
// Shader Parameters
//----------------------------------------------------------------------------------------
// Vertex attribute indices:
const int32 kAttribIndex_positions = 0;
const int32 kAttribIndex_texCoords = 1;

//----------------------------------------------------------------------------------------
// Texture Storage Parameters
//----------------------------------------------------------------------------------------
const int32 kSimTextureWidth = 512;
const int32 kSimTextureHeight = 512;
const float32 kDx = 1.0f / kSimTextureWidth; // Grid cell length

// Grid Layout Specification
template <int32 N>
struct Grid {
    vec2 worldOrigin;
    float32 cellLength;
    int32 textureWidth;    // Given in number of cells
    int32 textureHeight;   // Given in number of cells
    int32 textureUnit;     // Active texture unit
    GLenum internalFormat; // Storage type
    GLenum components;     // Color channels requested
    GLenum dataType;
    GLuint textureName[N]; // Used for binding GL textures
};
const int READ = 0;
const int WRITE = 1;

Grid<2> u_velocityGrid = {
        vec2(0, 0.5*kDx),      // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth + 1,  // textureWidth
        kSimTextureHeight,     // textureHeight
        0,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid<2> v_velocityGrid = {
        vec2(0.5*kDx, 0),      // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight + 1, // textureHeight
        1,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid<2> densityGrid = {
        0.5f*vec2(kDx, kDx),   // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight,     // textureHeight
        2,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid<2> pressureGrid = {
        0.5f*vec2(kDx, kDx),   // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight,     // textureHeight
        3,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};


class GpuSmokeSim2D : public GlfwOpenGlWindow {

public:
    ~GpuSmokeSim2D() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    GpuSmokeSim2D() = default; // Singleton. Prevent direct construction.

    GLuint framebuffer; // Framebuffer Object

    GLuint screenQuadVao;         // Vertex Array Object
    GLuint screenQuadVertBuffer;  // Vertex Buffer Object
    GLuint screenQuadIndexBuffer; // Element Buffer Object

    ShaderProgram shaderProgram_Advect;
    ShaderProgram shaderProgram_SceneRenderer;

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void keyInput(int key, int action, int mods);
    virtual void cleanup();

    void setFramebufferColorAttachment2D(GLuint framebuffer,
                                         GLuint textureId);

    void createTextureStorage();
    void initTextureData();
    void setupScreenQuadVboData();
    void setupShaderPrograms();
    void setShaderUniforms();
    void swapTextureNames(Grid<2> & grid);
    void advect(Grid<2> & dataGrid);
    void render(const Grid<2> & dataGrid);

    // TODO Dustin - Remove these after passing advect test:
    void fillTexturesWithData();

};
