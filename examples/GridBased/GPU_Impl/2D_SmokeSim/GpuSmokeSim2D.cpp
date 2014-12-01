#include "GpuSmokeSim2D.hpp"

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include <chrono>
#include <sstream>
using namespace std;
using namespace std::chrono;


//----------------------------------------------------------------------------------------
int main() {
    std::shared_ptr<GlfwOpenGlWindow> smokeDemo = GpuSmokeSim2D::getInstance();
    smokeDemo->create(kScreenWidth,
                      kScreenHeight,
                      "2D GPU Smoke Simulation",
                      kSecondsPerFrame);

    return 0;
}

//---------------------------------------------------------------------------------------
std::shared_ptr<GlfwOpenGlWindow> GpuSmokeSim2D::getInstance() {
    static GlfwOpenGlWindow * instance = new GpuSmokeSim2D();
    if (p_instance == nullptr) {
        p_instance = std::shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::init() {
    glGenFramebuffers(1, &framebuffer);

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

    createDepthStencilBufferStorage();


    // TODO Dustin - Remove this after passing advect tests:
        fillTexturesWithData();

    stencilFluidCells();

    glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass all fragments with stencil == 1.
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0x00); // Prevent writing to the stencil buffer.

    glClearColor(0.0, 0.0, 0.0, 1.0f);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/**
* Fill simulation textures with initial data.
*/
void GpuSmokeSim2D::initTextureData() {
    // Make large enough so each texture can be be filled with data.
    float32 data[ (kSimTextureWidth+1) * (kSimTextureHeight+1)];
    for(float32 &f : data) {
        f = 0;
    }

    //-- u_velocityGrid:
    for(int i(0); i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, u_velocityGrid.textureName[i]);
        glTexSubImage2D(GL_TEXTURE_2D,
                        0, 0, 0,
                        u_velocityGrid.textureWidth,
                        u_velocityGrid.textureHeight,
                        u_velocityGrid.components,
                        u_velocityGrid.dataType,
                        data);
    }

    //-- v_velocityGrid:
    for(int i(0); i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, v_velocityGrid.textureName[i]);
        glTexSubImage2D(GL_TEXTURE_2D,
                0, 0, 0,
                v_velocityGrid.textureWidth,
                v_velocityGrid.textureHeight,
                v_velocityGrid.components,
                v_velocityGrid.dataType,
                data);
    }

    //-- densityGrid:
    for(int i(0); i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, densityGrid.textureName[i]);
        glTexSubImage2D(GL_TEXTURE_2D,
                0, 0, 0,
                densityGrid.textureWidth,
                densityGrid.textureHeight,
                densityGrid.components,
                densityGrid.dataType,
                data);
    }

    //-- pressureGrid:
    glBindTexture(GL_TEXTURE_2D, pressureGrid.textureName[READ]);
    glTexSubImage2D(GL_TEXTURE_2D,
            0, 0, 0,
            pressureGrid.textureWidth,
            pressureGrid.textureHeight,
            pressureGrid.components,
            pressureGrid.dataType,
            data);

    //-- rhsGrid:
    glBindTexture(GL_TEXTURE_2D, rhsGrid.textureName[READ]);
    glTexSubImage2D(GL_TEXTURE_2D,
            0, 0, 0,
            rhsGrid.textureWidth,
            rhsGrid.textureHeight,
            rhsGrid.components,
            rhsGrid.dataType,
            data);

    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::fillTexturesWithData() {

    //-- v_velocityGrid:
    {
        float32 data[ v_velocityGrid.textureWidth * v_velocityGrid.textureHeight ];

//        // Make v_velocity point upwards for each grid cell:
//        for (float32 &f : data) {
//                f = 0.5f * kDx;
//        }

                // TODO Dustin - Remove after testing:
                // Make every other row constant, in order to test computeRHS()
                const int width = v_velocityGrid.textureWidth;
                float value;
                for(int i(0); i < v_velocityGrid.textureHeight; ++i) {
                    for(int j(0); j < width; ++j) {
                        if (i % 10 == 0) {
                            value = 10.0f;
                        } else {
                            value = 0.0f;
                        }
                        data[i * width + j] = value;
                    }
                }

        for (int i(0); i < 2; ++i) {
            glBindTexture(GL_TEXTURE_2D, v_velocityGrid.textureName[i]);
            glTexSubImage2D(GL_TEXTURE_2D,
                    0, 0, 0,
                    v_velocityGrid.textureWidth,
                    v_velocityGrid.textureHeight,
                    v_velocityGrid.components,
                    v_velocityGrid.dataType,
                    data);
        }
    }

    //-- densityGrid:
    {
        uint32 width = densityGrid.textureWidth;
        uint32 height = densityGrid.textureHeight;
        float32 data[width * height];
        for (float32 &f : data) {
            f = 0.0f;
        }

        // Put some density values into densityGrid:
        for (int i(0); i < 10; ++i) {
            for (int j(100); j < densityGrid.textureWidth-100; ++j) {
                data[i * width + j] = 1.0f;
            }
        }
        for (int i(0); i < 2; ++i) {

            glBindTexture(GL_TEXTURE_2D, densityGrid.textureName[i]);
            glTexSubImage2D(GL_TEXTURE_2D,
                    0, 0, 0,
                    densityGrid.textureWidth,
                    densityGrid.textureHeight,
                    densityGrid.components,
                    densityGrid.dataType,
                    data);
        }
    }

    //-- cellTypeGrid:
    {

        const float32 FLUID = 0.0f;
        const float32 SOLID = 1.0f;

        const uint32 width = cellTypeGrid.textureWidth;
        const uint32 height = cellTypeGrid.textureHeight;
        float32 data[height][width];

        // Initialize all cells to FLUID first.
        for(int row(0); row < height; ++row) {
            for(int col(0); col < width; ++col) {
                data[row][col] = FLUID;
            }
        }

        // Set Left and Right borders to SOLID.
        for(int row(0); row < height; ++row) {
            data[row][0] = SOLID;
            data[row][(width-1)] = SOLID;
        }

        // Set Top and Bottom borders to SOLID.
        for(int col(0); col < width; ++col) {
            data[height-1][col] = SOLID;
            data[0][col] = SOLID;
        }

        // Add a few SOLID cells to interior.
        // Create a Solid Box near center of grid
        int32 mid_col = kSimTextureWidth / 2;
        int32 mid_row = kSimTextureHeight / 2;
        int32 halfWidthSpan = (kSimTextureWidth*0.2f) * 0.5f;
        int32 halfHeightSpan = (kSimTextureWidth*0.05f) * 0.5f;
        for (int32 i(-halfHeightSpan); i < halfHeightSpan; ++i) {
            for (int32 j(-halfWidthSpan); j < halfWidthSpan; ++j) {
                data[mid_row + i][mid_col + j] = SOLID;
            }
        }

        glBindTexture(GL_TEXTURE_2D, cellTypeGrid.textureName[READ]);
        glTexSubImage2D(GL_TEXTURE_2D,
                0, 0, 0,
                cellTypeGrid.textureWidth,
                cellTypeGrid.textureHeight,
                cellTypeGrid.components,
                cellTypeGrid.dataType,
                data);

        glBindTexture(GL_TEXTURE_2D, 0);

    }

    CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::setupScreenQuadVboData() {
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

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::setupShaderPrograms() {
    shaderProgram_Advect.loadFromFile (
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/Advect.vs",
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/Advect.fs");

    shaderProgram_ComputeRHS.loadFromFile(
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/ComputeRHS.vs",
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/ComputeRHS.fs");

    shaderProgram_StencilFluidCells.loadFromFile(
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/StencilFluidCells.vs",
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/StencilFluidCells.fs");

    shaderProgram_SceneRenderer.loadFromFile(
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/ScreenQuad.vs",
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/ScreenQuad.fs");

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::setShaderUniforms() {
    //-- shaderProgram_Advect:
    {
        //-- u_velocityGrid:
        {
            shaderProgram_Advect.setUniform("u_velocityGrid.worldOrigin",
                    u_velocityGrid.worldOrigin);

            shaderProgram_Advect.setUniform("u_velocityGrid.cellLength",
                    u_velocityGrid.cellLength);

            shaderProgram_Advect.setUniform("u_velocityGrid.textureWidth",
                    u_velocityGrid.textureWidth);

            shaderProgram_Advect.setUniform("u_velocityGrid.textureHeight",
                    u_velocityGrid.textureHeight);

            shaderProgram_Advect.setUniform("u_velocityGrid.textureUnit",
                    u_velocityGrid.textureUnit);
        }

        //-- v_velocityGrid:
        {
            shaderProgram_Advect.setUniform("v_velocityGrid.worldOrigin",
                    v_velocityGrid.worldOrigin);

            shaderProgram_Advect.setUniform("v_velocityGrid.cellLength",
                    v_velocityGrid.cellLength);

            shaderProgram_Advect.setUniform("v_velocityGrid.textureWidth",
                    v_velocityGrid.textureWidth);

            shaderProgram_Advect.setUniform("v_velocityGrid.textureHeight",
                    v_velocityGrid.textureHeight);

            shaderProgram_Advect.setUniform("v_velocityGrid.textureUnit",
                    v_velocityGrid.textureUnit);

        }
        shaderProgram_Advect.setUniform("timeStep", kDt);
    }

    //-- shaderProgram_ComputeRHS:
    {
        //-- u_velocityGrid:
        {
            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.worldOrigin",
                    u_velocityGrid.worldOrigin);

            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.cellLength",
                    u_velocityGrid.cellLength);

            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.textureWidth",
                    u_velocityGrid.textureWidth);

            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.textureHeight",
                    u_velocityGrid.textureHeight);

            shaderProgram_ComputeRHS.setUniform("u_velocityGrid.textureUnit",
                    u_velocityGrid.textureUnit);
        }

        //-- v_velocityGrid:
        {
            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.worldOrigin",
                    v_velocityGrid.worldOrigin);

            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.cellLength",
                    v_velocityGrid.cellLength);

            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.textureWidth",
                    v_velocityGrid.textureWidth);

            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.textureHeight",
                    v_velocityGrid.textureHeight);

            shaderProgram_ComputeRHS.setUniform("v_velocityGrid.textureUnit",
                    v_velocityGrid.textureUnit);

        }
        
        //-- cellTypeGrid:
        {
            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.worldOrigin",
                    cellTypeGrid.worldOrigin);

            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.cellLength",
                    cellTypeGrid.cellLength);

            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.textureWidth",
                    cellTypeGrid.textureWidth);

            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.textureHeight",
                    cellTypeGrid.textureHeight);

            shaderProgram_ComputeRHS.setUniform("cellTypeGrid.textureUnit",
                    cellTypeGrid.textureUnit);

        }
        shaderProgram_ComputeRHS.setUniform("timeStep", kDt);
        shaderProgram_ComputeRHS.setUniform("density", kDensity);
        shaderProgram_ComputeRHS.setUniform("u_solid", 0.0f);
        shaderProgram_ComputeRHS.setUniform("v_solid", 0.0f);
    }

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::createDepthStencilBufferStorage() {
    glGenRenderbuffers(1, &depth_stencil_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
            kSimTextureWidth, kSimTextureHeight);


    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::createTextureStorage() {

    //-- u_velocityGrid textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &u_velocityGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_2D, u_velocityGrid.textureName[i]);

            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         u_velocityGrid.internalFormat,
                         u_velocityGrid.textureWidth,
                         u_velocityGrid.textureHeight,
                         0,
                         u_velocityGrid.components,
                         u_velocityGrid.dataType,
                         NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_2D, 0);

            CHECK_GL_ERRORS;
        }
    }

    //-- v_velocityGrid textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &v_velocityGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_2D, v_velocityGrid.textureName[i]);

            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         v_velocityGrid.internalFormat,
                         v_velocityGrid.textureWidth,
                         v_velocityGrid.textureHeight,
                         0,
                         v_velocityGrid.components,
                         v_velocityGrid.dataType,
                         NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_2D, 0);

            CHECK_GL_ERRORS;
        }
    }

    //-- densityGrid textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &densityGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_2D, densityGrid.textureName[i]);

            glTexImage2D(GL_TEXTURE_2D,
                         0,
                         densityGrid.internalFormat,
                         densityGrid.textureWidth,
                         densityGrid.textureHeight,
                         0,
                         densityGrid.components,
                         densityGrid.dataType,
                         NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            const GLfloat borderColor[4] = {0.0, 0.0, 0.0, 0.0};
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            glBindTexture(GL_TEXTURE_2D, 0);

            CHECK_GL_ERRORS;
        }
    }

    //-- pressureGrid texture:
    {
        glGenTextures(1, &pressureGrid.textureName[READ]);
        glBindTexture(GL_TEXTURE_2D, pressureGrid.textureName[READ]);

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     pressureGrid.internalFormat,
                     pressureGrid.textureWidth,
                     pressureGrid.textureHeight,
                     0,
                     pressureGrid.components,
                     pressureGrid.dataType,
                     NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        CHECK_GL_ERRORS;
    }

    //-- rhsGrid texture:
    {
        glGenTextures(1, &rhsGrid.textureName[READ]);
        glBindTexture(GL_TEXTURE_2D, rhsGrid.textureName[READ]);

        glTexImage2D(GL_TEXTURE_2D,
                0,
                rhsGrid.internalFormat,
                rhsGrid.textureWidth,
                rhsGrid.textureHeight,
                0,
                rhsGrid.components,
                rhsGrid.dataType,
                NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        CHECK_GL_ERRORS;
    }

    //-- cellTypeGrid texture:
    {
        glGenTextures(1, &cellTypeGrid.textureName[READ]);
        glBindTexture(GL_TEXTURE_2D, cellTypeGrid.textureName[READ]);

        glTexImage2D(GL_TEXTURE_2D,
                0,
                cellTypeGrid.internalFormat,
                cellTypeGrid.textureWidth,
                cellTypeGrid.textureHeight,
                0,
                cellTypeGrid.components,
                cellTypeGrid.dataType,
                NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);

        CHECK_GL_ERRORS;
    }

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::checkFramebufferCompleteness() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        stringstream error;
        error << "Error. Framebuffer not complete.";
        error << " ErroCode: " << status;
        throw Rigid3DException(error.str());
    }
}

//----------------------------------------------------------------------------------------
/**
* Assumes 'framebuffer' is currently bound using:
*       glBindFramebuffer(framebufferType, framebuffer).
*
* @param framebufferType' - one of GL_FRAMEBUFFER, GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER.
*/
void GpuSmokeSim2D::setFramebufferColorAttachment2D(
        GLenum framebufferType,
        GLuint framebuffer,
        GLuint textureName)
{
    glFramebufferTexture2D(framebufferType, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            textureName, 0);

    #ifdef DEBUG
        checkFramebufferCompleteness();
    #endif

    CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::swapTextureNames(Grid & grid) {
    GLuint tmp = grid.textureName[READ];
    grid.textureName[READ] = grid.textureName[WRITE];
    grid.textureName[WRITE] = tmp;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::advect(Grid & dataGrid) {
    // Process only the texels belonging to dataGrid:
    glViewport(0, 0, dataGrid.textureWidth, dataGrid.textureHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    setFramebufferColorAttachment2D(GL_FRAMEBUFFER, framebuffer,
            dataGrid.textureName[WRITE]);

    glActiveTexture(GL_TEXTURE0 + u_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_2D, u_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + v_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_2D, v_velocityGrid.textureName[READ]);

    glActiveTexture(GL_TEXTURE0 + dataGrid.textureUnit);
    glBindTexture(GL_TEXTURE_2D, dataGrid.textureName[READ]);

    shaderProgram_Advect.setUniform("dataGrid.worldOrigin",
            dataGrid.worldOrigin);

    shaderProgram_Advect.setUniform("dataGrid.cellLength",
            dataGrid.cellLength);

    shaderProgram_Advect.setUniform("dataGrid.textureWidth",
            dataGrid.textureWidth);

    shaderProgram_Advect.setUniform("dataGrid.textureHeight",
            dataGrid.textureHeight);

    shaderProgram_Advect.setUniform("dataGrid.textureUnit",
            dataGrid.textureUnit);

    //-- Render screen quad:
    {
        glBindVertexArray(screenQuadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);

        shaderProgram_Advect.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
        shaderProgram_Advect.disable();
    }


    //-- Reset back to defaults:
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::computeRHS() {
    // Attach rhsGrid texture to framebuffer for writing.
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    setFramebufferColorAttachment2D(GL_FRAMEBUFFER, framebuffer,
            rhsGrid.textureName[0]);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
            depth_stencil_rbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
            depth_stencil_rbo);

    #ifdef DEBUG
        checkFramebufferCompleteness();
    #endif

    // Set u_velocityGrid texture for reading.
    glActiveTexture(GL_TEXTURE0 + u_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_2D, u_velocityGrid.textureName[READ]);

    // Set v_velocityGrid texture for reading.
    glActiveTexture(GL_TEXTURE0 + v_velocityGrid.textureUnit);
    glBindTexture(GL_TEXTURE_2D, v_velocityGrid.textureName[READ]);

    // Set cellTypeGrid texture for reading.
    glActiveTexture(GL_TEXTURE0 + cellTypeGrid.textureUnit);
    glBindTexture(GL_TEXTURE_2D, cellTypeGrid.textureName[READ]);

    glViewport(0, 0, kSimTextureWidth, kSimTextureHeight);

    // Only process fluid cells, which have stencil value = 1.
    glEnable(GL_STENCIL_TEST);

        glBindVertexArray(screenQuadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);

        shaderProgram_ComputeRHS.enable();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
        shaderProgram_ComputeRHS.disable();

    glDisable(GL_STENCIL_TEST);


    //-- Restore defaults:
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Write 0's to stencil buffer at location of solid cells.
// Write 1's to stencil buffer at location of fluid cells.
void GpuSmokeSim2D::stencilFluidCells() {

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
            depth_stencil_rbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
            depth_stencil_rbo);

    // Framebuffer needs a color attachment to be complete, but we won't render to it.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            cellTypeGrid.textureName[0], 0);

    #ifdef DEBUG
        checkFramebufferCompleteness();
    #endif

    glViewport(0, 0, kSimTextureWidth, kSimTextureHeight);

    // Clear the depth and stencil buffers.
    glClearStencil(1); // Sets all cells to Fluid initially.
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    glEnable(GL_STENCIL_TEST);

        // Each non-discarded fragment sets stencil value to 0.
        glStencilFunc(GL_ALWAYS, 0, 0xFF);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF); // Write to stencil buffer
        // Disable writing to the color and depth buffers
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        glActiveTexture(GL_TEXTURE0 + cellTypeGrid.textureUnit);
        glBindTexture(GL_TEXTURE_2D, cellTypeGrid.textureName[0]);
        shaderProgram_StencilFluidCells.setUniform("u_textureUnit", cellTypeGrid.textureUnit);

        glBindVertexArray(screenQuadVao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);

        shaderProgram_StencilFluidCells.enable();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
        shaderProgram_StencilFluidCells.disable();

    glDisable(GL_STENCIL_TEST);


    //-- Restore defaults:
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    CHECK_GL_ERRORS;

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::render(const Grid &dataGrid) {
    // Use default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0 + dataGrid.textureUnit);
    glBindTexture(GL_TEXTURE_2D, dataGrid.textureName[READ]);
    shaderProgram_SceneRenderer.setUniform("u_textureUnit", dataGrid.textureUnit);

    glBindVertexArray(screenQuadVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);

    shaderProgram_SceneRenderer.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    shaderProgram_SceneRenderer.disable();


    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::logic() {

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::inspectGridData(Grid & grid) {
    float * data = new float[grid.textureWidth * grid.textureHeight];
    for(int i(0); i < grid.textureHeight; ++i) {
        for(int j(0); j < grid.textureWidth; ++j) {
            data[i * grid.textureWidth + j] = 0.0f;
        }
    }

    glFinish();
    glBindTexture(GL_TEXTURE_2D, densityGrid.textureName[READ]);
    glGetTexImage(GL_TEXTURE_2D, 0, densityGrid.components, densityGrid.dataType, data);


    delete [] data;
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::draw() {
    // 1. Advect Velocity
    // 2. Advect Density
    // 3. Compute and Apply Forces
    // 4. Compute RHS (use tmpTexture)
    // 5. Compute Pressure
    // 6. Subtract Pressure Gradient from velocity (use tmpTexture)
    // 7. Render

//    advect(u_velocityGrid);
//    advect(v_velocityGrid);
//    swapTextureNames(u_velocityGrid);
//    swapTextureNames(v_velocityGrid);
//
//    advect(densityGrid);
//    swapTextureNames(densityGrid);

    computeRHS();


    // Render to entire window
    glViewport(0, 0, defaultFramebufferWidth(), defaultFramebufferHeight());
//    render(densityGrid);

    // TODO Dustin - Remove this after testing:
    render(rhsGrid);

    CHECK_GL_ERRORS;
}
//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::keyInput(int key, int action, int mods) {

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::cleanup() {
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
