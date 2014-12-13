#include "GpuSmokeSim3D.hpp"

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include "VolumeRenderer.hpp"

#include <glm/glm.hpp>
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

            // Set constant density near bottom layers
            if(layer > 0 && layer < 10) {
                glClearColor(layer,0,0,0);
            } else {
                glClearColor(0,0,0,0);
            }
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
            glClearColor(kTemp_0,0,0,0);
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

    //-- rhsGrid:
    glViewport(0, 0, rhsGrid.textureWidth, rhsGrid.textureHeight);
    for(int layer = 0; layer < rhsGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer,
                rhsGrid.textureName[READ], layer);

        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    //-- cellTypeGrid:
    glViewport(0, 0, cellTypeGrid.textureWidth, cellTypeGrid.textureHeight);
    for(int layer = 0; layer < cellTypeGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer,
                cellTypeGrid.textureName[READ], layer);

        // Inside of volume is all Fluid (Solid = 1, Fluid = 0).
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
    shaderProgram_Advect.loadFromFile (
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/Advect.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/Advect.fs");

    shaderProgram_BuoyantForce.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/BuoyantForce.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/BuoyantForce.fs");

    shaderProgram_ComputeRHS.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ComputeRHS.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ComputeRHS.fs");

    shaderProgram_SceneRenderer.loadFromFile(
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ScreenQuad.vs",
            "examples/GridBased/GPU_Impl/3D_SmokeSim/shaders/ScreenQuad.fs");

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
        }
        //-- v_velocityGrid:
        {
            shaderProgram_Advect.setUniform("v_velocityGrid.worldOrigin",
                    v_velocityGrid.worldOrigin);
            shaderProgram_Advect.setUniform("v_velocityGrid.cellLength",
                    v_velocityGrid.cellLength);
            shaderProgram_Advect.setUniform("v_velocityGrid.textureUnit",
                    v_velocityGrid.textureUnit);

        }
        //-- w_velocityGrid:
        {
            shaderProgram_Advect.setUniform("w_velocityGrid.worldOrigin",
                    w_velocityGrid.worldOrigin);
            shaderProgram_Advect.setUniform("w_velocityGrid.cellLength",
                    w_velocityGrid.cellLength);
            shaderProgram_Advect.setUniform("w_velocityGrid.textureUnit",
                    w_velocityGrid.textureUnit);
        }
        shaderProgram_Advect.setUniform("timeStep", kDt);
    }



    //-- shaderProgram_ComputeRHS:
    {
        //-- u_velocityGrid:
        {
            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.cellLength",
                    u_velocityGrid.cellLength);
            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.textureWidth",
                    u_velocityGrid.textureWidth);
            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.textureHeight",
                    u_velocityGrid.textureHeight);
            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.textureDepth",
                    u_velocityGrid.textureDepth);
            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.textureUnit",
                    u_velocityGrid.textureUnit);
        }
        //-- v_velocityGrid:
        {

            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.cellLength",
                    v_velocityGrid.cellLength);
            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.textureWidth",
                    v_velocityGrid.textureWidth);
            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.textureHeight",
                    v_velocityGrid.textureHeight);
            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.textureDepth",
                    v_velocityGrid.textureDepth);
            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.textureUnit",
                    v_velocityGrid.textureUnit);

        }
        //-- w_velocityGrid:
        {

            shaderProgram_ComputeRHS.setUniform("w_velocityGrid.cellLength",
                    w_velocityGrid.cellLength);
            shaderProgram_ComputeRHS.setUniform("w_velocityGrid.textureWidth",
                    w_velocityGrid.textureWidth);
            shaderProgram_ComputeRHS.setUniform("w_velocityGrid.textureHeight",
                    w_velocityGrid.textureHeight);
            shaderProgram_ComputeRHS.setUniform("w_velocityGrid.textureDepth",
                    w_velocityGrid.textureDepth);
            shaderProgram_ComputeRHS.setUniform("w_velocityGrid.textureUnit",
                    w_velocityGrid.textureUnit);

        }
        //-- cellTypeGrid:
        {

            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.cellLength",
                    cellTypeGrid.cellLength);
            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.cellLength",
                    cellTypeGrid.cellLength);
            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.textureWidth",
                    cellTypeGrid.textureWidth);
            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.textureHeight",
                    cellTypeGrid.textureHeight);
            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.textureDepth",
                    cellTypeGrid.textureDepth);
            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.textureUnit",
                    cellTypeGrid.textureUnit);

        }
        shaderProgram_ComputeRHS.setUniform("timeStep", kDt);
        shaderProgram_ComputeRHS.setUniform("density", kDensity);
        shaderProgram_ComputeRHS.setUniform("u_solid", 0.0f);
        shaderProgram_ComputeRHS.setUniform("v_solid", 0.0f);
        shaderProgram_ComputeRHS.setUniform("w_solid", 0.0f);
    }


    //-- shaderProgram_BuoyantForce:
    {
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.worldOrigin",
                w_velocityGrid.worldOrigin);
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.cellLength",
                w_velocityGrid.cellLength);
        shaderProgram_BuoyantForce.setUniform("w_velocityGrid.textureUnit",
                w_velocityGrid.textureUnit);
        shaderProgram_BuoyantForce.setUniform("w_velocityDepth",
                float(w_velocityGrid.textureDepth));

        shaderProgram_BuoyantForce.setUniform("densityGrid.worldOrigin",
                densityGrid.worldOrigin);
        shaderProgram_BuoyantForce.setUniform("densityGrid.cellLength",
                densityGrid.cellLength);
        shaderProgram_BuoyantForce.setUniform("densityGrid.textureUnit",
                densityGrid.textureUnit);

        shaderProgram_BuoyantForce.setUniform("temperatureGrid.worldOrigin",
                temperatureGrid.worldOrigin);
        shaderProgram_BuoyantForce.setUniform("temperatureGrid.cellLength",
                temperatureGrid.cellLength);
        shaderProgram_BuoyantForce.setUniform("temperatureGrid.textureUnit",
                temperatureGrid.textureUnit);

        shaderProgram_BuoyantForce.setUniform("Kd", kBuoyant_d);
        shaderProgram_BuoyantForce.setUniform("Kt", kBuoyant_t);
        shaderProgram_BuoyantForce.setUniform("Temp0", kTemp_0);
        shaderProgram_BuoyantForce.setUniform("timeStep", kDt);

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
        glGenTextures(1, &pressureGrid.textureName[READ]);
        glBindTexture(GL_TEXTURE_3D, pressureGrid.textureName[READ]);

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

        glBindTexture(GL_TEXTURE_3D, 0);
        CHECK_GL_ERRORS;
    }

    //-- rhsGrid texture:
    {
        glGenTextures(1, &rhsGrid.textureName[READ]);
        glBindTexture(GL_TEXTURE_3D, rhsGrid.textureName[READ]);

        glTexImage3D(GL_TEXTURE_3D,
                0,
                rhsGrid.internalFormat,
                rhsGrid.textureWidth,
                rhsGrid.textureHeight,
                rhsGrid.textureDepth,
                0,
                rhsGrid.components,
                rhsGrid.dataType,
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

    shaderProgram_Advect.setUniform("dataGridDepth", float(dataGrid.textureDepth));

    // Loop over all layers of the 3D texture, processing one at a time:
    for(uint32 layer(0); layer < dataGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer, dataGrid.textureName[WRITE], layer);

        shaderProgram_Advect.setUniform("dataGridLayer", layer);

        renderScreenQuad(shaderProgram_Advect);
    }

    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim3D::computeRHS() {

    // Process only the texels belonging to dataGrid:
    glViewport(0, 0, rhsGrid.textureWidth, rhsGrid.textureHeight);

    glActiveTexture(GL_TEXTURE0 + u_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, u_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + v_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, v_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + w_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, w_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + cellTypeGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, cellTypeGrid.textureName[READ]);

    // Loop over all layers of the 3D texture, processing one at a time:
    for (uint32 layer = 0; layer < rhsGrid.textureDepth; ++layer) {
        // Attach rhsGrid texture to framebuffer for writing.
        bindFramebufferWithAttachments(framebuffer, rhsGrid.textureName[0], layer);

        shaderProgram_ComputeRHS.setUniform("currentLayer", layer);

        renderScreenQuad(shaderProgram_ComputeRHS);
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

    float max_value = 0;
    float min_value = INT_MAX;
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
    glViewport(0, 0, w_velocityGrid.textureWidth, w_velocityGrid.textureHeight);

    glActiveTexture(GL_TEXTURE0 + densityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, densityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + temperatureGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, temperatureGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0  + w_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_3D, w_velocityGrid.textureName[READ]);


    // Loop over all layers of the 3D texture, processing one at a time:
    for(uint32 layer(0); layer < w_velocityGrid.textureDepth; ++layer) {
        bindFramebufferWithAttachments(framebuffer, w_velocityGrid.textureName[WRITE], layer);

        shaderProgram_BuoyantForce.setUniform("currentLayer", layer);

        renderScreenQuad(shaderProgram_Advect);
    }

    swapTextureNames(w_velocityGrid);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;

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
    // 1. Advect Velocity
    // 2. Advect Density
    // 3. Compute and Apply Forces
    // 4. Compute RHS (use tmpTexture)
    // 5. Compute Pressure
    // 6. Subtract Pressure Gradient from velocity (use tmpTexture)
    // 7. Render

    advectQuantities();
//    addBuoyantForce();
    computeRHS();

//    inspectGridData(densityGrid);
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
    glDeleteTextures(2, densityGrid.textureName);
    glDeleteTextures(1, &pressureGrid.textureName[0]);
    glDeleteTextures(1, &cellTypeGrid.textureName[0]);

    CHECK_GL_ERRORS;
}
