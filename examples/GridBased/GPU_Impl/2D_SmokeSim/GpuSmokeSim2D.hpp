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

#include <chrono>
using namespace std::chrono;

//----------------------------------------------------------------------------------------
// Simulation Parameters
//----------------------------------------------------------------------------------------
const float32 kSecondsPerFrame = 1/100.0f;
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
struct Grid {
    vec2 worldOrigin;
    float32 cellLength;
    int32 textureWidth;    // Given in number of cells
    int32 textureHeight;   // Given in number of cells
    int32 textureUnit;     // Active texture unit
    GLenum internalFormat; // Storage type
    GLenum components;     // Color channels requested
    GLenum dataType;
    GLuint textureName[2]; // Used for binding GL textures, [0]=READ, [1]=WRITE.
};
const int READ = 0;
const int WRITE = 1;

Grid u_velocityGrid = {
        vec2(0, 0.5*kDx),      // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth + 1,  // textureWidth
        kSimTextureHeight,     // textureHeight
        0,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid v_velocityGrid = {
        vec2(0.5*kDx, 0),      // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight + 1, // textureHeight
        1,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid densityGrid = {
        0.5f*vec2(kDx, kDx),   // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight,     // textureHeight
        2,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid pressureGrid = {
        0.5f*vec2(kDx, kDx),   // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight,     // textureHeight
        3,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

// For determining which cells are solid or fluid.
Grid cellTypeGrid = {
        0.5f*vec2(kDx, kDx),   // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight,     // textureHeight
        4,                     // textureUnit
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

    void setFramebufferColorAttachment2D(GLenum framebufferType,
                                         GLuint framebuffer,
                                         GLuint textureName);

    void createTextureStorage();
    void initTextureData();
    void setupScreenQuadVboData();
    void setupShaderPrograms();
    void setShaderUniforms();
    void swapTextureNames(Grid & grid);
    void advect(Grid & dataGrid);

    void render(const Grid & dataGrid);


    // TODO Dustin - Remove these after testing is complete:
        void fillTexturesWithData();
        void inspectGridData(Grid & grid);
};
