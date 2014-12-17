#include "GpuSmokeSim3D.hpp"

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include "VolumeRenderer.hpp"
#include "matrix_transform_2d.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;


#include <chrono>
using namespace std::chrono;
#include <sstream>
using namespace std;


//----------------------------------------------------------------------------------------
int main() {
    std::shared_ptr<GlfwOpenGlWindow> smokeDemo = GpuSmokeSim3D::getInstance();
    smokeDemo->create(kScreenWidth,
                      kScreenHeight,
                      "GPU Smoke Simulation",
                      kSecondsPerFrame);

    return 0;
}

//---------------------------------------------------------------------------------------
std::shared_ptr<GlfwOpenGlWindow> GpuSmokeSim3D::getInstance() {
    static GlfwOpenGlWindow * instance = new GpuSmokeSim3D();
    if (p_instance == nullptr) {
        p_instance = std::shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}
//---------------------------------------------------------------------------------------
GpuSmokeSim3D::~GpuSmokeSim3D() {
    delete volumeRenderer;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::init() {
    volumeRenderer = new VolumeRenderer(kSimTextureWidth,
                                        kSimTextureHeight,
                                        kSimTextureDepth,
                                        uint(defaultFramebufferWidth()),
                                        uint(defaultFramebufferHeight()));

    glGenFramebuffers(1, &framebuffer);

    createDepthBufferStorage();

    glGenVertexArrays(1, &screenQuadVao);
    glBindVertexArray(screenQuadVao);
        glEnableVertexAttribArray(kAttribIndex_positions);
        glEnableVertexAttribArray(kAttribIndex_texCoords);
    glBindVertexArray(0);


    setupScreenQuadVboData();

    createTextureStorage();

    initTextureData();

    setupShaderPrograms();

    setShaderUniforms();

    initCamera();

    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::initCamera() {
    camera.setNearZDistance(0.1f);
    camera.setFarZDistance(100.0f);
    camera.setPosition(0.7, 0.8, 1.5);
    camera.lookAt(0, 0, 0);
}

//----------------------------------------------------------------------------------------
// TODO Dustin - Can remove this after testing:
static void fillGridWithData(const Grid & grid) {
    int width = grid.textureWidth;
    int height = grid.textureHeight;
    int depth = grid.textureDepth;

    float * data = new float[width * height * depth];

    for (int i(0); i < width; ++i) {
        for (int j(0); j < height; ++j) {
            for (int k(0); k < depth; ++k) {
                data[k * (width * height) + j * width + i] = k+1;
            }
        }
    }

    glBindTexture(GL_TEXTURE_3D, grid.textureName[READ]);
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, grid.textureWidth, grid.textureHeight,
            grid.textureDepth, grid.components, grid.dataType, data);


    glBindTexture(GL_TEXTURE_3D, 0);
    delete [] data;

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/**
* Fill simulation textures with initial data.
*/
void GpuSmokeSim3D::initTextureData() {

    //-- u_velocityGrid:
    glViewport(0, 0, u_velocityGrid.textureWidth, u_velocityGrid.textureHeight);
    for(int i(0); i < 2; ++i) {
        for(int layer = 0; layer < u_velocityGrid.textureDepth; ++layer) {
            bindFramebufferWithAttachments(framebuffer,
                    u_velocityGrid.textureName[i], layer);

            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }


    //-- v_velocityGrid:
    glViewport(0, 0, v_velocityGrid.textureWidth, v_velocityGrid.textureHeight);
    for(int i(0); i < 2; ++i) {
        for(int layer = 0; layer < v_velocityGrid.textureDepth; ++layer) {
            bindFramebufferWithAttachments(framebuffer,
                    v_velocityGrid.textureName[i], layer);

            glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }


    //-- w_velocityGrid:
    // Initial value is constant upwards at every cell
    glViewport(0, 0, w_velocityGrid.textureWidth, w_velocityGrid.textureHeight);
    for(int i(0); i < 2; ++i) {
        for(int layer = 0; layer < w_velocityGrid.textureDepth; ++layer) {
            bindFramebufferWithAttachments(framebuffer,
                    w_velocityGrid.textureName[i], layer);

            glClearColor(0,0,0,0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    //-- densityGrid:
    glViewport(0, 0, densityGrid.textureWidth, densityGrid.textureHeight);
    for(int i(0); i < 2; ++i) {
        for(int layer = 0; layer < densityGrid.textureDepth; ++layer) {
            bindFramebufferWithAttachments(framebuffer,
                densityGrid.textureName[i], layer);

            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    //-- temperatureGrid:
    glViewport(0, 0, temperatureGrid.textureWidth, temperatureGrid.textureHeight);
    for(int i(0); i < 2; ++i) {
        for(int layer = 0; layer < temperatureGrid.textureDepth; ++layer) {
            bindFramebufferWithAttachments(framebuffer,
                    temperatureGrid.textureName[i], layer);

            // Set all cells to ambient temperature kTemp_0
            glClearColor(kTemp_0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    //-- pressureGrid:
    glViewport(0, 0, pressureGrid.textureWidth, pressureGrid.textureHeight);
    for(int layer = 0; layer < pressureGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer,
                pressureGrid.textureName[READ], layer );

        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    //-- divergenceGrid:
    glViewport(0, 0, divergenceGrid.textureWidth, divergenceGrid.textureHeight);
    for(int layer = 0; layer < divergenceGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer,
                divergenceGrid.textureName[READ], layer);

        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    //-- cellTypeGrid:
    glViewport(0, 0, cellTypeGrid.textureWidth, cellTypeGrid.textureHeight);
    for(int layer = 0; layer < cellTypeGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer,
                cellTypeGrid.textureName[READ], layer);

        // Inside volume is all Fluid (Solid = 1, Fluid = 0).
        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    //-- Reset defaults:
    glViewport(0,0, defaultFramebufferWidth(), defaultFramebufferHeight());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::setupScreenQuadVboData() {
    glBindVertexArray(screenQuadVao);

    glGenBuffers(1, &screenQuadVertBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVertBuffer);

    //-- Create vertex data for screen quad
    {
        float32 vertexData[] = {
            //  Position      Texture-Coords
            -1.0f,  1.0f,    0.0f, 1.0f,   // Top-left
             1.0f,  1.0f,    1.0f, 1.0f,   // Top-right
             1.0f, -1.0f,    1.0f, 0.0f,   // Bottom-right
            -1.0f, -1.0f,    0.0f, 0.0f    // Bottom-left
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

        int32 elementsPerVertex;
        int32 stride;
        int32 offsetToFirstElement;

        //-- Position Data:
        elementsPerVertex = 2;
        stride = 4*sizeof(float32);
        offsetToFirstElement = 0;
        glVertexAttribPointer(kAttribIndex_positions, elementsPerVertex, GL_FLOAT,
                GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

        //-- Texture Coordinate Data:
        elementsPerVertex = 2;
        stride = 4*sizeof(float32);
        offsetToFirstElement = 2*sizeof(float32);
        glVertexAttribPointer(kAttribIndex_texCoords, elementsPerVertex, GL_FLOAT,
                GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));
    }

    //-- Create element buffer data for indices:
    {
        glGenBuffers(1, &screenQuadIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);

        GLushort indices [] = {
                3,2,1, 0,3,1
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::setupShaderPrograms() {

    shaderProgram_SceneRenderer.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ForwardVertexData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ScreenQuad.fs");

    shaderProgram_Advect.loadFromFile (
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ForwardVertexData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/Advect.fs");

    shaderProgram_BuoyantForce.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ForwardVertexData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/BuoyantForce.fs");

    shaderProgram_ComputeDivergence.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ForwardVertexData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ComputeDivergence.fs");

    shaderProgram_InjectData.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/InjectData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/InjectData.fs");

    shaderProgram_PressureSolve.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ForwardVertexData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/PressureSolve.fs");

    shaderProgram_ProjectU.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ForwardVertexData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ProjectU.fs");

    shaderProgram_ProjectV.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ForwardVertexData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ProjectV.fs");

    shaderProgram_ProjectW.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ForwardVertexData.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ProjectW.fs");

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::setShaderUniforms() {
    //-- shaderProgram_Advect:
    {
        //-- u_velocityGrid:
        {
            shaderProgram_Advect.setUniform("u_velocityGrid.worldOrigin",
                    u_velocityGrid.worldOrigin);
            shaderProgram_Advect.setUniform("u_velocityGrid.cellLength",
                    u_velocityGrid.cellLength);
            shaderProgram_Advect.setUniform("u_velocityGrid.textureUnit",
                    u_velocityGrid.textureUnit);
            shaderProgram_Advect.setUniform("u_velocityGrid.textureWidth",
                    u_velocityGrid.textureWidth);
            shaderProgram_Advect.setUniform("u_velocityGrid.textureHeight",
                    u_velocityGrid.textureHeight);
            shaderProgram_Advect.setUniform("u_velocityGrid.textureDepth",
                    u_velocityGrid.textureDepth);
        }
        //-- v_velocityGrid:
        {
            shaderProgram_Advect.setUniform("v_velocityGrid.worldOrigin",
                    v_velocityGrid.worldOrigin);
            shaderProgram_Advect.setUniform("v_velocityGrid.cellLength",
                    v_velocityGrid.cellLength);
            shaderProgram_Advect.setUniform("v_velocityGrid.textureUnit",
                    v_velocityGrid.textureUnit);
            shaderProgram_Advect.setUniform("v_velocityGrid.textureWidth",
                    v_velocityGrid.textureWidth);
            shaderProgram_Advect.setUniform("v_velocityGrid.textureHeight",
                    v_velocityGrid.textureHeight);
            shaderProgram_Advect.setUniform("v_velocityGrid.textureDepth",
                    v_velocityGrid.textureDepth);

        }
        //-- w_velocityGrid:
        {
            shaderProgram_Advect.setUniform("w_velocityGrid.worldOrigin",
                    w_velocityGrid.worldOrigin);
            shaderProgram_Advect.setUniform("w_velocityGrid.cellLength",
                    w_velocityGrid.cellLength);
            shaderProgram_Advect.setUniform("w_velocityGrid.textureUnit",
                    w_velocityGrid.textureUnit);
            shaderProgram_Advect.setUniform("w_velocityGrid.textureWidth",
                    w_velocityGrid.textureWidth);
            shaderProgram_Advect.setUniform("w_velocityGrid.textureHeight",
                    w_velocityGrid.textureHeight);
            shaderProgram_Advect.setUniform("w_velocityGrid.textureDepth",
                    w_velocityGrid.textureDepth);
        }
        shaderProgram_Advect.setUniform("timeStep", kDt);
    }



    //-- shaderProgram_ComputeDivergence:
    {
        //-- u_velocityGrid:
        {
            shaderProgram_ComputeDivergence.setUniform("u_velocityGrid.cellLength",
                    u_velocityGrid.cellLength);
            shaderProgram_ComputeDivergence.setUniform("u_velocityGrid.textureWidth",
                    u_velocityGrid.textureWidth);
            shaderProgram_ComputeDivergence.setUniform("u_velocityGrid.textureHeight",
                    u_velocityGrid.textureHeight);
            shaderProgram_ComputeDivergence.setUniform("u_velocityGrid.textureDepth",
                    u_velocityGrid.textureDepth);
            shaderProgram_ComputeDivergence.setUniform("u_velocityGrid.textureUnit",
                    u_velocityGrid.textureUnit);
        }
        //-- v_velocityGrid:
        {

            shaderProgram_ComputeDivergence.setUniform("v_velocityGrid.cellLength",
                    v_velocityGrid.cellLength);
            shaderProgram_ComputeDivergence.setUniform("v_velocityGrid.textureWidth",
                    v_velocityGrid.textureWidth);
            shaderProgram_ComputeDivergence.setUniform("v_velocityGrid.textureHeight",
                    v_velocityGrid.textureHeight);
            shaderProgram_ComputeDivergence.setUniform("v_velocityGrid.textureDepth",
                    v_velocityGrid.textureDepth);
            shaderProgram_ComputeDivergence.setUniform("v_velocityGrid.textureUnit",
                    v_velocityGrid.textureUnit);

        }
        //-- w_velocityGrid:
        {

            shaderProgram_ComputeDivergence.setUniform("w_velocityGrid.cellLength",
                    w_velocityGrid.cellLength);
            shaderProgram_ComputeDivergence.setUniform("w_velocityGrid.textureWidth",
                    w_velocityGrid.textureWidth);
            shaderProgram_ComputeDivergence.setUniform("w_velocityGrid.textureHeight",
                    w_velocityGrid.textureHeight);
            shaderProgram_ComputeDivergence.setUniform("w_velocityGrid.textureDepth",
                    w_velocityGrid.textureDepth);
            shaderProgram_ComputeDivergence.setUniform("w_velocityGrid.textureUnit",
                    w_velocityGrid.textureUnit);

        }
        //-- cellTypeGrid:
        {

            shaderProgram_ComputeDivergence.setUniform("cellTypeGrid.cellLength",
                    cellTypeGrid.cellLength);
            shaderProgram_ComputeDivergence.setUniform("cellTypeGrid.cellLength",
                    cellTypeGrid.cellLength);
            shaderProgram_ComputeDivergence.setUniform("cellTypeGrid.textureWidth",
                    cellTypeGrid.textureWidth);
            shaderProgram_ComputeDivergence.setUniform("cellTypeGrid.textureHeight",
                    cellTypeGrid.textureHeight);
            shaderProgram_ComputeDivergence.setUniform("cellTypeGrid.textureDepth",
                    cellTypeGrid.textureDepth);
            shaderProgram_ComputeDivergence.setUniform("cellTypeGrid.textureUnit",
                    cellTypeGrid.textureUnit);

        }
        shaderProgram_ComputeDivergence.setUniform("timeStep", kDt);
        shaderProgram_ComputeDivergence.setUniform("density", kDensity);
        shaderProgram_ComputeDivergence.setUniform("u_solid", 0.0f);
        shaderProgram_ComputeDivergence.setUniform("v_solid", 0.0f);
        shaderProgram_ComputeDivergence.setUniform("w_solid", 0.0f);
    }


    //-- shaderProgram_BuoyantForce:
    {
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.worldOrigin",
                w_velocityGrid.worldOrigin);
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.cellLength",
                w_velocityGrid.cellLength);
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.textureUnit",
                w_velocityGrid.textureUnit);
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.textureWidth",
                w_velocityGrid.textureWidth);
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.textureHeight",
                w_velocityGrid.textureHeight);
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.textureDepth",
                w_velocityGrid.textureDepth);

        shaderProgram_BuoyantForce.setUniform("densityGrid.worldOrigin",
                densityGrid.worldOrigin);
        shaderProgram_BuoyantForce.setUniform("densityGrid.cellLength",
                densityGrid.cellLength);
        shaderProgram_BuoyantForce.setUniform("densityGrid.textureUnit",
                densityGrid.textureUnit);
        shaderProgram_BuoyantForce.setUniform("densityGrid.textureWidth",
                densityGrid.textureWidth);
        shaderProgram_BuoyantForce.setUniform("densityGrid.textureHeight",
                densityGrid.textureHeight);
        shaderProgram_BuoyantForce.setUniform("densityGrid.textureDepth",
                densityGrid.textureDepth);

        shaderProgram_BuoyantForce.setUniform("temperatureGrid.worldOrigin",
                temperatureGrid.worldOrigin);
        shaderProgram_BuoyantForce.setUniform("temperatureGrid.cellLength",
                temperatureGrid.cellLength);
        shaderProgram_BuoyantForce.setUniform("temperatureGrid.textureUnit",
                temperatureGrid.textureUnit);
        shaderProgram_BuoyantForce.setUniform("temperatureGrid.textureWidth",
                temperatureGrid.textureWidth);
        shaderProgram_BuoyantForce.setUniform("temperatureGrid.textureHeight",
                temperatureGrid.textureHeight);
        shaderProgram_BuoyantForce.setUniform("temperatureGrid.textureDepth",
                temperatureGrid.textureDepth);

        shaderProgram_BuoyantForce.setUniform("Kd", kBuoyant_d);
        shaderProgram_BuoyantForce.setUniform("Kt", kBuoyant_t);
        shaderProgram_BuoyantForce.setUniform("Temp0", kTemp_0);
        shaderProgram_BuoyantForce.setUniform("timeStep", kDt);

    }

    //-- shaderProgram_PressureSolve
    {
        shaderProgram_PressureSolve.setUniform("pressureGrid.textureUnit",
                pressureGrid.textureUnit);
        shaderProgram_PressureSolve.setUniform("pressureGrid.textureWidth",
                pressureGrid.textureWidth);
        shaderProgram_PressureSolve.setUniform("pressureGrid.textureHeight",
                pressureGrid.textureHeight);
        shaderProgram_PressureSolve.setUniform("pressureGrid.textureDepth",
                pressureGrid.textureDepth);

        shaderProgram_PressureSolve.setUniform("divergenceGrid.textureUnit",
                divergenceGrid.textureUnit);
        shaderProgram_PressureSolve.setUniform("divergenceGrid.textureWidth",
                divergenceGrid.textureWidth);
        shaderProgram_PressureSolve.setUniform("divergenceGrid.textureHeight",
                divergenceGrid.textureHeight);
        shaderProgram_PressureSolve.setUniform("divergenceGrid.textureDepth",
                divergenceGrid.textureDepth);

        shaderProgram_PressureSolve.setUniform("cellTypeGrid.textureUnit",
                cellTypeGrid.textureUnit);
        shaderProgram_PressureSolve.setUniform("cellTypeGrid.textureWidth",
                cellTypeGrid.textureWidth);
        shaderProgram_PressureSolve.setUniform("cellTypeGrid.textureHeight",
                cellTypeGrid.textureHeight);
        shaderProgram_PressureSolve.setUniform("cellTypeGrid.textureDepth",
                cellTypeGrid.textureDepth);

    }

    //-- shaderProgram_ProjectU
    {
        shaderProgram_ProjectU.setUniform("pressureGrid.textureWidth",
                pressureGrid.textureWidth);
        shaderProgram_ProjectU.setUniform("pressureGrid.textureHeight",
                pressureGrid.textureHeight);
        shaderProgram_ProjectU.setUniform("pressureGrid.textureDepth",
                pressureGrid.textureDepth);
        shaderProgram_ProjectU.setUniform("pressureGrid.textureUnit",
                pressureGrid.textureUnit);

        shaderProgram_ProjectU.setUniform("cellTypeGrid.textureWidth",
                cellTypeGrid.textureWidth);
        shaderProgram_ProjectU.setUniform("cellTypeGrid.textureHeight",
                cellTypeGrid.textureHeight);
        shaderProgram_ProjectU.setUniform("cellTypeGrid.textureDepth",
                cellTypeGrid.textureDepth);
        shaderProgram_ProjectU.setUniform("cellTypeGrid.textureUnit",
                cellTypeGrid.textureUnit);

        shaderProgram_ProjectU.setUniform("u_velocityGrid.textureWidth",
                u_velocityGrid.textureWidth);
        shaderProgram_ProjectU.setUniform("u_velocityGrid.textureHeight",
                u_velocityGrid.textureHeight);
        shaderProgram_ProjectU.setUniform("u_velocityGrid.textureDepth",
                u_velocityGrid.textureDepth);
        shaderProgram_ProjectU.setUniform("u_velocityGrid.textureUnit",
                u_velocityGrid.textureUnit);

        shaderProgram_ProjectU.setUniform("solid_velocity", 0.0f);
        shaderProgram_ProjectU.setUniform("scale", kDt/(kDensity*kDx));
    }

    //-- shaderProgram_ProjectV
    {
        shaderProgram_ProjectV.setUniform("pressureGrid.textureWidth",
                pressureGrid.textureWidth);
        shaderProgram_ProjectV.setUniform("pressureGrid.textureHeight",
                pressureGrid.textureHeight);
        shaderProgram_ProjectV.setUniform("pressureGrid.textureDepth",
                pressureGrid.textureDepth);
        shaderProgram_ProjectV.setUniform("pressureGrid.textureUnit",
                pressureGrid.textureUnit);

        shaderProgram_ProjectV.setUniform("cellTypeGrid.textureWidth",
                cellTypeGrid.textureWidth);
        shaderProgram_ProjectV.setUniform("cellTypeGrid.textureHeight",
                cellTypeGrid.textureHeight);
        shaderProgram_ProjectV.setUniform("cellTypeGrid.textureDepth",
                cellTypeGrid.textureDepth);
        shaderProgram_ProjectV.setUniform("cellTypeGrid.textureUnit",
                cellTypeGrid.textureUnit);

        shaderProgram_ProjectV.setUniform("v_velocityGrid.textureWidth",
                v_velocityGrid.textureWidth);
        shaderProgram_ProjectV.setUniform("v_velocityGrid.textureHeight",
                v_velocityGrid.textureHeight);
        shaderProgram_ProjectV.setUniform("v_velocityGrid.textureDepth",
                v_velocityGrid.textureDepth);
        shaderProgram_ProjectV.setUniform("v_velocityGrid.textureUnit",
                v_velocityGrid.textureUnit);

        shaderProgram_ProjectV.setUniform("solid_velocity", 0.0f);
        shaderProgram_ProjectV.setUniform("scale", kDt/(kDensity*kDx));
    }

    //-- shaderProgram_ProjectW
    {
        shaderProgram_ProjectW.setUniform("pressureGrid.textureWidth",
                pressureGrid.textureWidth);
        shaderProgram_ProjectW.setUniform("pressureGrid.textureHeight",
                pressureGrid.textureHeight);
        shaderProgram_ProjectW.setUniform("pressureGrid.textureDepth",
                pressureGrid.textureDepth);
        shaderProgram_ProjectW.setUniform("pressureGrid.textureUnit",
                pressureGrid.textureUnit);

        shaderProgram_ProjectW.setUniform("cellTypeGrid.textureWidth",
                cellTypeGrid.textureWidth);
        shaderProgram_ProjectW.setUniform("cellTypeGrid.textureHeight",
                cellTypeGrid.textureHeight);
        shaderProgram_ProjectW.setUniform("cellTypeGrid.textureDepth",
                cellTypeGrid.textureDepth);
        shaderProgram_ProjectW.setUniform("cellTypeGrid.textureUnit",
                cellTypeGrid.textureUnit);

        shaderProgram_ProjectW.setUniform("w_velocityGrid.textureWidth",
                w_velocityGrid.textureWidth);
        shaderProgram_ProjectW.setUniform("w_velocityGrid.textureHeight",
                w_velocityGrid.textureHeight);
        shaderProgram_ProjectW.setUniform("w_velocityGrid.textureDepth",
                w_velocityGrid.textureDepth);
        shaderProgram_ProjectW.setUniform("w_velocityGrid.textureUnit",
                w_velocityGrid.textureUnit);

        shaderProgram_ProjectW.setUniform("solid_velocity", 0.0f);
        shaderProgram_ProjectW.setUniform("scale", kDt/(kDensity*kDx));
    }

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::createDepthBufferStorage() {
    glGenRenderbuffers(1, &depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
            kSimTextureWidth, kSimTextureHeight);


    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::createTextureStorage() {

    //-- u_velocityGrid textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &u_velocityGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_3D, u_velocityGrid.textureName[i]);

            glTexImage3D(GL_TEXTURE_3D,
                         0,
                         u_velocityGrid.internalFormat,
                         u_velocityGrid.textureWidth,
                         u_velocityGrid.textureHeight,
                         u_velocityGrid.textureDepth,
                         0,
                         u_velocityGrid.components,
                         u_velocityGrid.dataType,
                         NULL);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_3D, 0);
            CHECK_GL_ERRORS;
        }
    }

    //-- v_velocityGrid textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &v_velocityGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_3D, v_velocityGrid.textureName[i]);

            glTexImage3D(GL_TEXTURE_3D,
                    0,
                    v_velocityGrid.internalFormat,
                    v_velocityGrid.textureWidth,
                    v_velocityGrid.textureHeight,
                    v_velocityGrid.textureDepth,
                    0,
                    v_velocityGrid.components,
                    v_velocityGrid.dataType,
                    NULL);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_3D, 0);
            CHECK_GL_ERRORS;
        }
    }

    //-- w_velocityGrid textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &w_velocityGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_3D, w_velocityGrid.textureName[i]);

            glTexImage3D(GL_TEXTURE_3D,
                    0,
                    w_velocityGrid.internalFormat,
                    w_velocityGrid.textureWidth,
                    w_velocityGrid.textureHeight,
                    w_velocityGrid.textureDepth,
                    0,
                    w_velocityGrid.components,
                    w_velocityGrid.dataType,
                    NULL);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_3D, 0);
            CHECK_GL_ERRORS;
        }
    }

    //-- densityGrid textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &densityGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_3D, densityGrid.textureName[i]);

            glTexImage3D(GL_TEXTURE_3D,
                         0,
                         densityGrid.internalFormat,
                         densityGrid.textureWidth,
                         densityGrid.textureHeight,
                         densityGrid.textureDepth,
                         0,
                         densityGrid.components,
                         densityGrid.dataType,
                         NULL);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

            // Assume zero density outside fluid:
            const GLfloat borderColor[4] = {0.0, 0.0, 0.0, 0.0};
            glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glBindTexture(GL_TEXTURE_3D, 0);
            CHECK_GL_ERRORS;
        }
    }

    //-- temperatureGrid textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &temperatureGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_3D, temperatureGrid.textureName[i]);

            glTexImage3D(GL_TEXTURE_3D,
                    0,
                    temperatureGrid.internalFormat,
                    temperatureGrid.textureWidth,
                    temperatureGrid.textureHeight,
                    temperatureGrid.textureDepth,
                    0,
                    temperatureGrid.components,
                    temperatureGrid.dataType,
                    NULL);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

            // Assume ambient temperature outside fluid:
            const GLfloat borderColor[4] = {kTemp_0, 0.0, 0.0, 0.0};
            glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glBindTexture(GL_TEXTURE_3D, 0);
            CHECK_GL_ERRORS;
        }
    }

    //-- pressureGrid texture:
    {
        for (int i = 0; i < 2; ++i) {
            glGenTextures(1, &pressureGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_3D, pressureGrid.textureName[i]);

            glTexImage3D(GL_TEXTURE_3D,
                    0,
                    pressureGrid.internalFormat,
                    pressureGrid.textureWidth,
                    pressureGrid.textureHeight,
                    pressureGrid.textureDepth,
                    0,
                    pressureGrid.components,
                    pressureGrid.dataType,
                    NULL);

            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
            const GLfloat borderColor[4] = {0.0, 0.0, 0.0, 0.0};
            glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);
        }

        glBindTexture(GL_TEXTURE_3D, 0);
        CHECK_GL_ERRORS;
    }

    //-- divergenceGrid texture:
    {
        glGenTextures(1, &divergenceGrid.textureName[READ]);
        glBindTexture(GL_TEXTURE_3D, divergenceGrid.textureName[READ]);

        glTexImage3D(GL_TEXTURE_3D,
                0,
                divergenceGrid.internalFormat,
                divergenceGrid.textureWidth,
                divergenceGrid.textureHeight,
                divergenceGrid.textureDepth,
                0,
                divergenceGrid.components,
                divergenceGrid.dataType,
                NULL);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        const GLfloat borderColor[4] = {0.0, 0.0, 0.0, 0.0};
        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindTexture(GL_TEXTURE_3D, 0);

        CHECK_GL_ERRORS;
    }

    //-- cellTypeGrid texture:
    {
        glGenTextures(1, &cellTypeGrid.textureName[READ]);
        glBindTexture(GL_TEXTURE_3D, cellTypeGrid.textureName[READ]);

        glTexImage3D(GL_TEXTURE_3D,
                0,
                cellTypeGrid.internalFormat,
                cellTypeGrid.textureWidth,
                cellTypeGrid.textureHeight,
                cellTypeGrid.textureDepth,
                0,
                cellTypeGrid.components,
                cellTypeGrid.dataType,
                NULL);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

        // Border color designates Solid cells (Solid = 1, Fluid = 0)
        // Using red channel only.
        const GLfloat borderColor[4] = {1.0, 0.0, 0.0, 0.0};
        glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, borderColor);


        glBindTexture(GL_TEXTURE_3D, 0);

        CHECK_GL_ERRORS;
    }

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::bindFramebufferWithAttachments(
        GLuint framebuffer,
        GLuint colorTextureName,
        GLuint layer,
        GLuint depthRenderBufferObject)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Attach a specific layer of 3D color texture
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTextureName,
            0, layer);

    // Optionally attach depthbuffer
    if (depthRenderBufferObject != 0) {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                depthRenderBufferObject);
    }

    CHECK_FRAMEBUFFER_COMPLETENESS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::swapTextureNames(Grid & grid) {
    GLuint tmp = grid.textureName[READ];
    grid.textureName[READ] = grid.textureName[WRITE];
    grid.textureName[WRITE] = tmp;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::renderScreenQuad(const ShaderProgram & shader) {
    glBindVertexArray(screenQuadVao);

    shader.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    shader.disable();

    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::advect(Grid & dataGrid) {

    // Process only the texels belonging to dataGrid:
    glViewport(0, 0, dataGrid.textureWidth, dataGrid.textureHeight);

    glActiveTexture(GL_TEXTURE0 + u_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, u_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + v_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, v_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0  + w_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, w_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + dataGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, dataGrid.textureName[READ]);

    shaderProgram_Advect.setUniform("dataGrid.textureUnit",
            dataGrid.textureUnit);
    shaderProgram_Advect.setUniform("dataGrid.worldOrigin",
            dataGrid.worldOrigin);
    shaderProgram_Advect.setUniform("dataGrid.cellLength",
            dataGrid.cellLength);
    shaderProgram_Advect.setUniform("dataGrid.textureWidth",
            dataGrid.textureWidth);
    shaderProgram_Advect.setUniform("dataGrid.textureHeight",
            dataGrid.textureHeight);
    shaderProgram_Advect.setUniform("dataGrid.textureDepth",
            dataGrid.textureDepth);

    // Loop over all layers of the 3D texture, processing one at a time:
    for(uint32 layer(0); layer < dataGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer, dataGrid.textureName[WRITE], layer);

        shaderProgram_Advect.setUniform("currentLayer", layer);

        renderScreenQuad(shaderProgram_Advect);
    }

    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::computeDivergence() {

    // Process only the texels belonging to dataGrid:
    glViewport(0, 0, divergenceGrid.textureWidth, divergenceGrid.textureHeight);

    glActiveTexture(GL_TEXTURE0 + u_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, u_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + v_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, v_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + w_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, w_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + cellTypeGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, cellTypeGrid.textureName[READ]);

    // Loop over all layers of the 3D texture, processing one at a time:
    for (uint32 layer = 0; layer < divergenceGrid.textureDepth; ++layer) {
        // Attach divergenceGrid texture to framebuffer for writing.
        bindFramebufferWithAttachments(framebuffer, divergenceGrid.textureName[0], layer);

        shaderProgram_ComputeDivergence.setUniform("currentLayer", layer);

        renderScreenQuad(shaderProgram_ComputeDivergence);
    }

    //-- Restore defaults:
    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::render(const Grid &dataGrid) {
    // Use default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0 + dataGrid.textureUnit);
    glBindTexture(GL_TEXTURE_2D, dataGrid.textureName[READ]);
    shaderProgram_SceneRenderer.setUniform("u_textureUnit", dataGrid.textureUnit);

    renderScreenQuad(shaderProgram_SceneRenderer);

    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::inspectGridData(Grid & grid) {
    int width = grid.textureWidth;
    int height = grid.textureHeight;
    int depth = grid.textureDepth;
    float * data = new float[width * height * depth];
    for(int i(0); i < width; ++i) {
        for(int j(0); j < height; ++j) {
            for(int k(0); k < depth; ++k) {
                data[k*(width*height) + j*width + i] = 0.0f;
            }
        }
    }

    glFinish();
    glBindTexture(GL_TEXTURE_3D, grid.textureName[READ]);
    glGetTexImage(GL_TEXTURE_3D, 0, grid.components, grid.dataType, data);

    float max_value = FLT_MIN;
    float min_value = FLT_MAX;
    float value;
    for(int i(0); i < width; ++i) {
        for(int j(0); j < height; ++j) {
            for(int k(0); k < depth; ++k) {
                value = data[k*(width*height) + j*width + i];
                max_value = glm::max(max_value, value);
                min_value = glm::min(min_value, value);
            }
        }
    }
    cout << "max_value: " << max_value << endl;
    cout << "min_value: " << min_value << endl;


    delete [] data;
    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}
//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::addBuoyantForce() {

    glActiveTexture(GL_TEXTURE0 + densityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, densityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + temperatureGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, temperatureGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0  + w_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, w_velocityGrid.textureName[READ]);

    glViewport(0, 0, w_velocityGrid.textureWidth, w_velocityGrid.textureHeight);

    // Loop over all layers of the 3D texture, processing one at a time:
    for(uint32 layer(0); layer < w_velocityGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer, w_velocityGrid.textureName[WRITE], layer);

        shaderProgram_BuoyantForce.setUniform("currentLayer", layer);

        renderScreenQuad(shaderProgram_BuoyantForce);
    }

    swapTextureNames(w_velocityGrid);

    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}
//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::injectDensityAndTemperature() {

    float width = 20;
    float height = 20;
    float depth = 10;

    //-- densityGrid:
    {
        shaderProgram_InjectData.setUniform("value", 20.0f);

        vec3 scaleFactor;
        scaleFactor.x = width / densityGrid.textureWidth;
        scaleFactor.y = height / densityGrid.textureHeight;
        scaleFactor.z = 1.0f;
        mat4 scaleMatrix = glm::scale(mat4(), scaleFactor);
        shaderProgram_InjectData.setUniform("modelMatrix", scaleMatrix);

        shaderProgram_InjectData.setUniform("dataGrid.textureWidth",
                densityGrid.textureWidth);
        shaderProgram_InjectData.setUniform("dataGrid.textureHeight",
                densityGrid.textureHeight);
        shaderProgram_InjectData.setUniform("dataGrid.textureDepth",
                densityGrid.textureDepth);
        shaderProgram_InjectData.setUniform("dataGrid.textureUnit",
                densityGrid.textureUnit);

        glActiveTexture(GL_TEXTURE0 + densityGrid.textureUnit);
        glBindTexture(GL_TEXTURE_3D, densityGrid.textureName[READ]);

        glViewport(0, 0, densityGrid.textureWidth, densityGrid.textureHeight);

        for (int layer = 0; layer < depth; ++layer) {
            bindFramebufferWithAttachments(framebuffer,
                    densityGrid.textureName[WRITE], layer);

            shaderProgram_InjectData.setUniform("currentLayer", float(layer));

            renderScreenQuad(shaderProgram_InjectData);
        }
    }
    swapTextureNames(densityGrid);

    //-- temperatureGrid:
    {
        shaderProgram_InjectData.setUniform("value", 200.0f);
        vec3 scaleFactor;
        scaleFactor.x = width / temperatureGrid.textureWidth;
        scaleFactor.y = height / temperatureGrid.textureHeight;
        scaleFactor.z = 1.0f;
        shaderProgram_InjectData.setUniform("modelMatrix", glm::scale(mat4(), scaleFactor));

        shaderProgram_InjectData.setUniform("dataGrid.textureWidth",
                temperatureGrid.textureWidth);
        shaderProgram_InjectData.setUniform("dataGrid.textureHeight",
                temperatureGrid.textureHeight);
        shaderProgram_InjectData.setUniform("dataGrid.textureDepth",
                temperatureGrid.textureDepth);
        shaderProgram_InjectData.setUniform("dataGrid.textureUnit",
                temperatureGrid.textureUnit);

        glActiveTexture(GL_TEXTURE0 + temperatureGrid.textureUnit);
        glBindTexture(GL_TEXTURE_3D, temperatureGrid.textureName[READ]);

        glViewport(0, 0, temperatureGrid.textureWidth, temperatureGrid.textureHeight);

        for (int layer = 0; layer < depth; ++layer) {
            bindFramebufferWithAttachments(framebuffer,
                    temperatureGrid.textureName[WRITE], layer);

            shaderProgram_InjectData.setUniform("currentLayer", float(layer));

            renderScreenQuad(shaderProgram_InjectData);
        }
    }
    swapTextureNames(temperatureGrid);

    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::computePressure() {

    glViewport(0, 0, pressureGrid.textureWidth, pressureGrid.textureHeight);

    // Zero out all pressures values, which will be the initial guess for Jacobi
    // iteration.
    for (int i = 0; i < 2; ++i) {
        for (int layer = 0; layer < pressureGrid.textureDepth; ++layer) {
            bindFramebufferWithAttachments(framebuffer, pressureGrid.textureName[i],
                    layer);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }

    glActiveTexture(GL_TEXTURE0 + pressureGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, pressureGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + divergenceGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, divergenceGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + cellTypeGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, cellTypeGrid.textureName[READ]);

    // Do Jacobi iterations:
    for (int iteration = 0; iteration < kJacobiIterations; ++iteration) {

        // Process each layer of pressureGrid one at a time:
        for (uint layer = 0; layer < pressureGrid.textureDepth; ++layer) {
            bindFramebufferWithAttachments(framebuffer, pressureGrid.textureName[WRITE],
                    layer);

            shaderProgram_PressureSolve.setUniform("currentLayer", layer);

            renderScreenQuad(shaderProgram_PressureSolve);
        }

        swapTextureNames(pressureGrid);
    }

    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;

}

//----------------------------------------------------------------------------------------
/**
* Subtracts off the gradient of pressure in order to make the current velocity
* divergence free.
*/
void GpuSmokeSim3D::subtractPressureGradient(Grid & grid, ShaderProgram & shader) {
    glViewport(0, 0, grid.textureWidth, grid.textureHeight);

    glActiveTexture(GL_TEXTURE0 + pressureGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, pressureGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + cellTypeGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, cellTypeGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + grid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, grid.textureName[READ]);

    // Process each layer of grid, one at a time:
    for (uint layer = 0; layer < grid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer, grid.textureName[WRITE],
                layer);

        shader.setUniform("currentLayer", layer);

        renderScreenQuad(shader);
    }
    swapTextureNames(grid);

    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::projectVelocity() {
    subtractPressureGradient(u_velocityGrid, shaderProgram_ProjectU);
    subtractPressureGradient(v_velocityGrid, shaderProgram_ProjectV);
    subtractPressureGradient(w_velocityGrid, shaderProgram_ProjectW);
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::advectQuantities() {
    advect(u_velocityGrid);
    advect(v_velocityGrid);
    advect(w_velocityGrid);
    swapTextureNames(u_velocityGrid);
    swapTextureNames(v_velocityGrid);
    swapTextureNames(w_velocityGrid);

    advect(densityGrid);
    swapTextureNames(densityGrid);
    advect(temperatureGrid);
    swapTextureNames(temperatureGrid);
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::logic() {
    //  Advect Velocity
    //  Advect Density
    //  Advect Temperature
    //  Compute and Apply Forces
    //  Compute RHS
    //  Solve for Pressure
    //  Subtract Pressure Gradient from velocity
    //  Render

    static int counter = 0;
    if (counter < 2) {
        injectDensityAndTemperature();
        ++counter;
    }

    advectQuantities();
    addBuoyantForce();
    computeDivergence();
    computePressure();
    projectVelocity();

//    inspectGridData(w_velocityGrid);
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::draw() {
    // Render to entire window
    glViewport(0, 0, defaultFramebufferWidth(), defaultFramebufferHeight());

    volumeRenderer->draw(camera, 0.05, densityGrid.textureName[READ]);
}
//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::keyInput(int key, int action, int mods) {

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::cleanup() {
    cout << endl << "average time: " << timer.getAverageElapsedTime() << endl;

    glDeleteFramebuffers(1, &framebuffer);

    glDeleteBuffers(1, &screenQuadVertBuffer);
    glDeleteBuffers(1, &screenQuadIndexBuffer);
    glDeleteVertexArrays(1, &screenQuadVao);

    glDeleteTextures(2, u_velocityGrid.textureName);
    glDeleteTextures(2, v_velocityGrid.textureName);
    glDeleteTextures(2, w_velocityGrid.textureName);
    glDeleteTextures(2, densityGrid.textureName);
    glDeleteTextures(2, temperatureGrid.textureName);
    glDeleteTextures(1, &pressureGrid.textureName[0]);
    glDeleteTextures(1, &cellTypeGrid.textureName[0]);

    CHECK_GL_ERRORS;
}
