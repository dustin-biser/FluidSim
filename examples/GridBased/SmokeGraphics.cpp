#include "SmokeGraphics.hpp"
#include "SmokeSim.hpp"

#include <vector>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

//----------------------------------------------------------------------------------------
void SmokeGraphics::init(const Grid<float32> & inkGrid) {
    setupShaders();
    setupVao();
    setupBufferData();
    setupUniforms();

    createInkTexture(inkGrid);

    // Render only the front face of geometry.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::setupShaders() {
    shaderProgram.loadFromFile("data/shaders/Smoke2DAdvect.vs",
                               "data/shaders/Smoke2DAdvect.fs");
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::setupVao() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(attribIndex_position);
    glEnableVertexAttribArray(attribIndex_texCoord);

    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::setupBufferData() {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    // Store vertexAttribute mappings, and bound element buffer
    glBindVertexArray(vao);

    // Create vertex data for a two triangle quad that will fill the screen:
    {
        float32 vertexData[] = {
            //  Position      Texcoords
            -1.0f,  1.0f,    0.0f, 1.0f, // Top-left
             1.0f,  1.0f,    1.0f, 1.0f, // Top-right
             1.0f, -1.0f,    1.0f, 0.0f, // Bottom-right
            -1.0f, -1.0f,    0.0f, 0.0f  // Bottom-left
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_DYNAMIC_DRAW);

        uint32 elementsPerVertex;
        uint32 stride;
        uint32 offsetToFirstElement;

        elementsPerVertex = 2;
        stride = 4*sizeof(float32);
        offsetToFirstElement = 0;
        glVertexAttribPointer(attribIndex_position, elementsPerVertex,
                GL_FLOAT, GL_FALSE, stride, (void *)offsetToFirstElement);

        elementsPerVertex = 2;
        stride = 4*sizeof(float32);
        offsetToFirstElement = 2*sizeof(float32);
        glVertexAttribPointer(attribIndex_texCoord, elementsPerVertex,
                GL_FLOAT, GL_FALSE, stride, (void *)offsetToFirstElement);
    }

    // Create element buffer data for indices:
    {
        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        GLushort indices [] = {
            3,2,1, 0,3,1
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::setupUniforms() {
    shaderProgram.setUniform("u_inkColor", inkColor);

    // sampler2D texInk will use TextureUnit 0
    shaderProgram.setUniform("u_tex2DInk", 0);
    
    CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void SmokeGraphics::createInkTexture(const Grid<float32> & inkGrid) {
    //--Setup ink texture.
    // The ink texture will represent a grid of size (kGridWidth, kGridHeight).
    // Type: R16
    // Red Channel - ink density/opacity
    {
        glGenTextures(1, &tex2D_ink);

        // tex2D_ink will be referenced via TextureUnit0 in the fragment shader.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex2D_ink);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        // Internally store texture as R16 - one channel of half-floats:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, inkGrid.width(), inkGrid.height(),
                     0, GL_RED, GL_FLOAT, inkGrid.data());

    }

    // Unbind the GL_TEXTURE_2D target.
    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::uploadTextureData(const Grid<float32> & inkGrid) {
    glBindTexture(GL_TEXTURE_2D, tex2D_ink);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, inkGrid.width(), inkGrid.height(), GL_RED,
            GL_FLOAT, inkGrid.data());

    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::draw() {
    glBindVertexArray(vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex2D_ink);

    shaderProgram.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    shaderProgram.disable();

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::cleanup() {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &tex2D_ink);

    CHECK_GL_ERRORS;
}
