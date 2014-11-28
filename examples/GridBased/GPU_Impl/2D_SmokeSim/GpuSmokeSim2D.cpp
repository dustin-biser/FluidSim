#include "GpuSmokeSim2D.hpp"

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include <sstream>
#include <iostream>
using namespace std;


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

    glClearColor(0.0, 0.0, 0.0, 1.0f);



    // TODO Dustin - Remove this after passing advect tests:
    fillTexturesWithData();


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
    for(int i(0); i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, pressureGrid.textureName[i]);
        glTexSubImage2D(GL_TEXTURE_2D,
                0, 0, 0,
                pressureGrid.textureWidth,
                pressureGrid.textureHeight,
                pressureGrid.components,
                pressureGrid.dataType,
                data);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::fillTexturesWithData() {

    //-- v_velocityGrid:
    {
        float32 data[ v_velocityGrid.textureWidth * v_velocityGrid.textureHeight ];

        // Make v_velocity point upwards for each grid cell:
        for (float32 &f : data) {
            f = 0.001f * kDx;
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

    glBindTexture(GL_TEXTURE_2D, 0);
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


    shaderProgram_SceneRenderer.loadFromFile(
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/ScreenQuad.vs",
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/ScreenQuad.fs");

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::setShaderUniforms() {
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

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::createTextureStorage() {

    //-- u_velocity textures:
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

    //-- v_velocity textures:
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

    //-- Density textures:
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
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_2D, 0);

            CHECK_GL_ERRORS;
        }
    }

    //-- Pressure textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &pressureGrid.textureName[i]);
            glBindTexture(GL_TEXTURE_2D, pressureGrid.textureName[i]);

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
    }
}

//----------------------------------------------------------------------------------------
/**
* Assumes 'framebuffer' is currently bound using:
*       glBindFramebuffer(GL_FRAMEBUFFER, framebuffer).
*/
void GpuSmokeSim2D::setFramebufferColorAttachment2D(
        GLuint framebuffer,
        GLuint textureName)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            textureName, 0);

    #ifdef DEBUG
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            stringstream error;
            error << "Error. Framebuffer not complete.";
            error << " ErroCode: " << status;
            throw Rigid3DException(error.str());
        }
    #endif

    CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::swapTextureNames(Grid<2> & grid) {
    GLuint tmp = grid.textureName[READ];
    grid.textureName[READ] = grid.textureName[WRITE];
    grid.textureName[WRITE] = tmp;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::advect(Grid<2> & dataGrid) {
    // Process only the texels belonging to dataGrid:
    glViewport(0, 0, dataGrid.textureWidth, dataGrid.textureHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    setFramebufferColorAttachment2D(framebuffer, dataGrid.textureName[WRITE]);

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
void GpuSmokeSim2D::render(const Grid<2> &dataGrid) {
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
//    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
//
//    glViewport(0, 0, 512, 512);
//
//    float data[3];
//    glReadPixels(0, 0, 1, 1, GL_RGB, GL_FLOAT, data);
//
//    cout << "(R,G,B) = (" << data[0] << ", " << data[1] << ", " << data[2] << ")" << endl;

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::draw() {

    // Only process texture elements
//    advect(u_velocityGrid);
//    advect(v_velocityGrid);
//    swapTextureNames(u_velocityGrid);
//    swapTextureNames(v_velocityGrid);

    advect(densityGrid);
    swapTextureNames(densityGrid);

    // 1. Advect Velocity
    // 2. Advect Density
    // 3. Compute and Apply Forces
    // 5. Compute RHS (use tmpTexture)
    // 4. Compute Pressure
    // 5. Subtract Pressure Gradient from velocity (use tmpTexture)

    // Render to entire window
    glViewport(0, 0, kScreenWidth, kScreenHeight);
    render(densityGrid);

    CHECK_GL_ERRORS;
}
//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::keyInput(int key, int action, int mods) {

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::cleanup() {
}
