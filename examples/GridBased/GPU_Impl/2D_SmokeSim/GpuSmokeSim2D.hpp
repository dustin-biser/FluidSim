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
const float32 kDt = 0.008;
const int32 solver_iterations = 60;
const int32 kScreenWidth = 1024;
const int32 kScreenHeight = 768;

//----------------------------------------------------------------------------------------
// Texture Storage Parameters
//----------------------------------------------------------------------------------------
const int32 kSimTextureWidth = 512;
const int32 kSimTextureHeight = 512;
const GLenum kVelocityTextureFormat = GL_RG;
const GLenum kDensityTextureFormat = GL_RED;
const GLenum kPressureTextureFormat = GL_RED;
const GLenum kTmpTextureR32Format = GL_RED;

//----------------------------------------------------------------------------------------
// Shader Parameters
//----------------------------------------------------------------------------------------
// Vertex attribute indices:
const int32 kAttribIndex_positions = 0;
const int32 kAttribIndex_texCoords = 1;

//----------------------------------------------------------------------------------------
// Fluid Parameters
//----------------------------------------------------------------------------------------
const float32 temp_0 = 273.0f; // Ambient Temperature in Kelvin
const float32 kBuoyant_d = 0.8f;// Density coefficient for buoyant force.
const float32 kBuoyant_t = 0.34f; // Temperature coefficient for buoyant force.
const float32 kDensity = 1.0f;

//----------------------------------------------------------------------------------------
// Grid Parameters
//----------------------------------------------------------------------------------------
// Note: Keep KGridWidth a multiple of 4 so texture alignment is optimal.
const int32 kGridWidth = 100;
const int32 kGridHeight = 100;
const float32 kDx = 1.0f/kGridWidth; // Grid cell length in meters.
const float32 inv_kDx = 1.0f/kDx;
const float32 u_solid = 0.0f; // horizontal velocity of solid boundaries.
const float32 v_solid = 0.0f; // vertical velocity of solid boundaries.


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

    ShaderProgram shaderProgram_Advection;
    ShaderProgram shaderProgram_SceneRenderer;

    // Store two versions of each texture. One for rendering to and one for
    // reading from
    GLuint velocityTexture[2]; // RG32
    GLuint densityTexture[2];  // Red32
    GLuint pressureTexture[2]; // Red32

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void keyInput(int key, int action, int mods);
    virtual void cleanup();

    void setFramebufferColorAttachment2D(GLuint framebuffer,
                                         GLuint textureId);

    void createTextureStorage();
    void setupScreenQuadVboData();
    void setupShaderPrograms();
    void advectVelocity();
    void render();


    // TODO Dustin - Remove this after passing tests:
        GLuint tmpTexture_RGB;    // RG32
        void renderToTexture(GLuint texture);
        void renderTextureToScreen(GLuint texture);
        ShaderProgram shaderProgram_tmp;
};
