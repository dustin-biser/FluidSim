#include "SmokeGraphics3D.hpp"
#include "SmokeSim3D.hpp"

#include <vector>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

//----------------------------------------------------------------------------------------
void SmokeGraphics3D::init(Camera * camera) {
    this->camera = camera;

    setupShaders();
    setupVao();
    setupBufferData();
    setupCamera();
    setupUniforms();


    // Render only the front face of geometry.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics3D::setupCamera() {
    camera->setFieldOfViewY(45.0f);
    camera->setAspectRatio(1.0f);
    camera->setNearZDistance(0.1f);
    camera->setFarZDistance(100.0f);
    camera->setPosition(1.0, 1.0, 2.0);
    camera->lookAt(0, 0, 0);
}

//----------------------------------------------------------------------------------------
void SmokeGraphics3D::setupShaders() {
    shaderProgram.loadFromFile("data/shaders/LineRender.vs",
                               "data/shaders/LineRender.fs");
}

//----------------------------------------------------------------------------------------
void SmokeGraphics3D::setupVao() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Enable Vertex Position Attribute Array
    glEnableVertexAttribArray(position_attribIndex);

    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics3D::setupBufferData() {

    //  Cube vertex offsets in model-space
    float32 cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,    // 0 Left Bottom Back
         0.5f, -0.5f, -0.5f,    // 1 Right Bottom Back

        -0.5f, -0.5f,  0.5f,    // 2 Left Bottom Front
         0.5f, -0.5f,  0.5f,    // 3 Right Bottom Front

        -0.5f,  0.5f, -0.5f,    // 4 Left Top Back
         0.5f,  0.5f, -0.5f,    // 5 Right Top Back

        -0.5f,  0.5f,  0.5f,    // 6 Left Top Front
         0.5f,  0.5f,  0.5f,    // 7 Right Top Front
    };

    // Cube indices for GL_LINES
    GLushort indices [] = {
        2,3,3,1,1,0,0,2,   // Bottom Face
        6,7,7,5,5,4,4,6,   // Top Face
        2,6,    // Left Front Vertical
        3,7,    // Right Front Vertical
        0,4,    // Left Back Vertical
        1,5,    // Right Back Vertical

    };

    // Store vertexAttribute mappings, and bound element buffer
    glBindVertexArray(vao);

    // Upload Vertex Position Data:
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Upload Index Data:
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    //-- Vertex Position Attribute Mapping:
    int32 elementsPerVertex = 3;
    int32 stride = 0;
    int32 offsetToFirstElement = 0;
    glVertexAttribPointer(position_attribIndex, elementsPerVertex,
            GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics3D::setupUniforms() {
    shaderProgram.setUniform("ModelViewMatrix", camera->getViewMatrix());
    shaderProgram.setUniform("ProjectionMatrix", camera->getProjectionMatrix());
    shaderProgram.setUniform("u_color", vec4(0.0, 0.0, 0.0, 1.0));

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics3D::draw() {
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

    shaderProgram.enable();
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
    shaderProgram.disable();

    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics3D::cleanup() {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteVertexArrays(1, &vao);

    CHECK_GL_ERRORS;
}
