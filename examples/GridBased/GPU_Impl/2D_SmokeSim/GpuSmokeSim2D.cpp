#include "GpuSmokeSim2D.hpp"

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include <iostream>
using namespace std;


//----------------------------------------------------------------------------------------
int main() {
    std::shared_ptr<GlfwOpenGlWindow> smokeDemo = GpuSmokeSim2D::getInstance();
    smokeDemo->create(kScreenWidth,
                      kScreenHeight,
                      "2D GPU Smoke Simulation",
                      1/60.0f);

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

    setupShaderPrograms();

    glClearColor(0.0, 0.0, 0.0, 1.0f);

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
//    shaderProgram_Advection.loadFromFile (
//            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/Advection.vs",
//            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/Advection.fs");


    shaderProgram_SceneRenderer.loadFromFile(
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/ScreenQuad.vs",
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/ScreenQuad.fs");

    shaderProgram_tmp.loadFromFile(
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/WriteOutData.vs",
            "examples/GridBased/GPU_Impl/2D_SmokeSim/shaders/WriteOutData.fs");
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::createTextureStorage() {

    //-- Velocity textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &velocityTexture[i]);
            glBindTexture(GL_TEXTURE_2D, velocityTexture[i]);

            glTexImage2D(GL_TEXTURE_2D, 0, kVelocityTextureFormat, kSimTextureWidth,
                    kSimTextureHeight, 0, kVelocityTextureFormat, GL_FLOAT, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D, 0);

            CHECK_GL_ERRORS;
        }
    }

    //-- Density textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &densityTexture[i]);
            glBindTexture(GL_TEXTURE_2D, densityTexture[i]);

            glTexImage2D(GL_TEXTURE_2D, 0, kDensityTextureFormat, kSimTextureWidth,
                    kSimTextureHeight, 0, kDensityTextureFormat, GL_FLOAT, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D, 0);

            CHECK_GL_ERRORS;
        }
    }

    //-- Pressure textures:
    {
        for(int i(0); i < 2; ++i) {
            glGenTextures(1, &pressureTexture[i]);
            glBindTexture(GL_TEXTURE_2D, pressureTexture[i]);

            glTexImage2D(GL_TEXTURE_2D, 0, kPressureTextureFormat, kSimTextureWidth,
                    kSimTextureHeight, 0, kPressureTextureFormat, GL_FLOAT, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D, 0);

            CHECK_GL_ERRORS;
        }
    }

    // TODO Dustin - remove this if not using tmpTextures
    //-- tmpTexture_R32 texture:
    {
        glGenTextures(1, &tmpTexture_RGB);
        glBindTexture(GL_TEXTURE_2D, tmpTexture_RGB);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, kSimTextureWidth,
                kSimTextureHeight, 0, GL_RGB, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        CHECK_GL_ERRORS;
    }
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::setFramebufferColorAttachment2D(
        GLuint framebuffer,
        GLuint textureId)
{
    GLint prevBoundDrawFramebuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevBoundDrawFramebuffer);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Attach textureId as COLOR_ATTACHMENT_0
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            textureId, 0);

    #ifdef DEBUG
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw Rigid3DException("Error. Framebuffer not complete.");
        }
    #endif

    glBindFramebuffer(GL_FRAMEBUFFER, prevBoundDrawFramebuffer);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/**
* Read from velocityTexture[0] and write to velocityTexture[1].
*/
void GpuSmokeSim2D::advectVelocity() {

    setFramebufferColorAttachment2D(framebuffer, velocityTexture[1]);

    glBindVertexArray(screenQuadVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocityTexture[0]);

    // Set shader uniform to sample from texture unit 0
    shaderProgram_Advection.setUniform("u_velocityTexture", 0);

    shaderProgram_Advection.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    shaderProgram_Advection.disable();

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::render() {
    // Use default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tmpTexture_RGB);
    shaderProgram_SceneRenderer.setUniform("u_textureUnit", 0);

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
void GpuSmokeSim2D::renderToTexture(GLuint texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    setFramebufferColorAttachment2D(framebuffer, tmpTexture_RGB);

    // Set color to blue
    shaderProgram_tmp.setUniform("u_color", vec4(0.5f,0.5f,1.0f,1.0f));

    glBindVertexArray(screenQuadVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);

    shaderProgram_tmp.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    shaderProgram_tmp.disable();


    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::renderTextureToScreen(GLuint texture) {
    // Use default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tmpTexture_RGB);
    shaderProgram_SceneRenderer.setUniform("u_textureUnit", 0);

    glBindVertexArray(screenQuadVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIndexBuffer);

    shaderProgram_SceneRenderer.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    shaderProgram_SceneRenderer.disable();


    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::logic() {
//    static bool firstRun = true;
//    if (firstRun) {
//        firstRun = false;
//        return;
//    }
//
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
    glViewport(0, 0, kSimTextureWidth, kSimTextureHeight);
    renderToTexture(tmpTexture_RGB);

    // 1. Advect Velocity
    // 2. Advect Density
    // 3. Compute and Apply Forces
    // 5. Compute RHS (use tmpTexture)
    // 4. Compute Pressure
    // 5. Subtract Pressure Gradient from velocity (use tmpTexture)

    // Render to entire window
    glViewport(0, 0, kScreenWidth, kScreenHeight);
//    render();
    renderTextureToScreen(tmpTexture_RGB);

    CHECK_GL_ERRORS;
}
//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::keyInput(int key, int action, int mods) {

}

//----------------------------------------------------------------------------------------
void GpuSmokeSim2D::cleanup() {
}
