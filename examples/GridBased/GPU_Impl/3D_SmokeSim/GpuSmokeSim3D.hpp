/**
* @brief GpuSmokeSim3D.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/NumericTypes.hpp"
using namespace FluidSim;

#include "Utils/GlfwOpenGlWindow.hpp"

#include <Rigid3D/Graphics/ShaderProgram.hpp>
using namespace Rigid3D;

#include "Timer.hpp"

// Forward Declaration
class VolumeRenderer;


//----------------------------------------------------------------------------------------
// Simulation Parameters
//----------------------------------------------------------------------------------------
const float32 kSecondsPerFrame = 1/80.0f;
const float32 kDt = kSecondsPerFrame;//0.008;
const int32 solver_iterations = 60;
const int32 kScreenWidth = 1024;
const int32 kScreenHeight = 768;

//----------------------------------------------------------------------------------------
// Fluid Parameters
//----------------------------------------------------------------------------------------
const float32 kTemp_0 = 273.0f; // Ambient Temperature in Kelvin
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
const int32 kSimTextureWidth = 64;
const int32 kSimTextureHeight = 64;
const int32 kSimTextureDepth = 64;
const float32 kDx = 1.0f / kSimTextureWidth; // Grid cell length

// Grid Layout Specification
struct Grid {
    vec3 worldOrigin;
    float32 cellLength;
    int32 textureWidth;    // Given in number of cells
    int32 textureHeight;   // Given in number of cells
    int32 textureDepth;    // Given in number of cells
    GLint textureUnit;     // Active texture unit
    GLenum internalFormat; // Storage type
    GLenum components;     // Color channels requested
    GLenum dataType;
    GLuint textureName[2]; // Used for binding GL textures, [0]=READ, [1]=WRITE.
};
const int READ = 0;
const int WRITE = 1;

Grid u_velocityGrid = {
        0.5f*vec3(0,kDx,kDx),  // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth + 1,  // textureWidth
        kSimTextureHeight,     // textureHeight
        kSimTextureDepth,      // textureDepth
        0,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid v_velocityGrid = {
        0.5f*vec3(kDx,0,kDx),  // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight + 1, // textureHeight
        kSimTextureDepth,      // textureDepth
        1,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid w_velocityGrid = {
        0.5f*vec3(kDx,kDx,0),  // worldOrigin
        kDx,                   // cellLength
        kSimTextureWidth,      // textureWidth
        kSimTextureHeight,     // textureHeight
        kSimTextureDepth + 1,  // textureDepth
        2,                     // textureUnit
        GL_R16F,               // internalFormat
        GL_RED,                // components
        GL_FLOAT               // dataType
};

Grid densityGrid = {
        0.5f*vec3(kDx,kDx,kDx), // worldOrigin, at center of voxel.
        kDx,                    // cellLength
        kSimTextureWidth,       // textureWidth
        kSimTextureHeight,      // textureHeight
        kSimTextureDepth,       // textureHeight
        3,                      // textureUnit
        GL_R16F,                // internalFormat
        GL_RED,                 // components
        GL_FLOAT                // dataType
};

Grid temperatureGrid = {
        0.5f*vec3(kDx,kDx,kDx), // worldOrigin, at center of voxel.
        kDx,                    // cellLength
        kSimTextureWidth,       // textureWidth
        kSimTextureHeight,      // textureHeight
        kSimTextureDepth,       // textureHeight
        4,                      // textureUnit
        GL_R16F,                // internalFormat
        GL_RED,                 // components
        GL_FLOAT                // dataType
};

Grid pressureGrid = {
        0.5f*vec3(kDx,kDx,kDx), // worldOrigin, at center of voxel.
        kDx,                    // cellLength
        kSimTextureWidth,       // textureWidth
        kSimTextureHeight,      // textureHeight
        kSimTextureDepth,       // textureHeight
        5,                      // textureUnit
        GL_R16F,                // internalFormat
        GL_RED,                 // components
        GL_FLOAT                // dataType
};

// Store right-hand-side (RHS) of Poisson-Pressure Solve, Ap = b.
Grid rhsGrid = {
        0.5f*vec3(kDx,kDx,kDx), // worldOrigin, at center of voxel.
        kDx,                    // cellLength
        kSimTextureWidth,       // textureWidth
        kSimTextureHeight,      // textureHeight
        kSimTextureDepth,       // textureHeight
        6,                      // textureUnit
        GL_R16F,                // internalFormat
        GL_RED,                 // components
        GL_FLOAT                // dataType
};

// For determining which cells are solid or fluid.
Grid cellTypeGrid = {
        0.5f*vec3(kDx,kDx,kDx), // worldOrigin, at center of voxel.
        kDx,                    // cellLength
        kSimTextureWidth,       // textureWidth
        kSimTextureHeight,      // textureHeight
        kSimTextureDepth,       // textureHeight
        7,                      // textureUnit
        GL_R8,                  // internalFormat
        GL_RED,                 // components
        GL_FLOAT                // dataType
};

class GpuSmokeSim3D : public GlfwOpenGlWindow {

public:
    ~GpuSmokeSim3D();

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    GpuSmokeSim3D() = default; // Singleton. Prevent direct construction.

    VolumeRenderer * volumeRenderer;

    GLuint framebuffer; // Framebuffer Object
    GLuint depth_rbo;   // Renderbuffer Object for depth attachment

    GLuint screenQuadVao;         // Vertex Array Object
    GLuint screenQuadVertBuffer;  // Vertex Buffer Object
    GLuint screenQuadIndexBuffer; // Element Buffer Object

    ShaderProgram shaderProgram_Advect;
    ShaderProgram shaderProgram_BuoyantForce;
    ShaderProgram shaderProgram_ComputeRHS;
    ShaderProgram shaderProgram_SceneRenderer;
    ShaderProgram shaderProgram_InjectData;

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void keyInput(int key, int action, int mods);
    virtual void cleanup();

    void initCamera();
    void createDepthBufferStorage();
    void createTextureStorage();
    void initTextureData();
    void setupScreenQuadVboData();
    void setupShaderPrograms();
    void setShaderUniforms();
    void bindFramebufferWithAttachments(GLuint framebuffer,
                                        GLuint colorTextureName,
                                        GLuint layer,
                                        GLuint depthRenderBufferObject = 0);
    void renderScreenQuad(const ShaderProgram & shader);

    void swapTextureNames(Grid & grid);
    void advect(Grid & dataGrid);
    void advectQuantities();
    void addBuoyantForce();
    void computeRHS();
    void injectDensityAndTemperature();
    void render(const Grid & dataGrid);


    // TODO Dustin - Remove these after testing is complete:
        void inspectGridData(Grid & grid);
        Timer timer;
};
