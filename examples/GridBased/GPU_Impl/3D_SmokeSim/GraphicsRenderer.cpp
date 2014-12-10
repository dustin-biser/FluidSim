#include "GraphicsRenderer.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace std;

//----------------------------------------------------------------------------------------
GraphicsRenderer::GraphicsRenderer(
        uint32 framebufferWidth,
        uint32 framebufferHeight,
        Camera * camera)
{
    this->camera = camera;

    setupShaders();
    setupVao();
    setupBoundingCubeVertexData();
    setupCamera();
    setupShaderUniforms();


    // Render only the front face of geometry.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.1, 0.1, 0.1, 1.0);

    glViewport(0, 0, framebufferWidth, framebufferHeight);

    CHECK_GL_ERRORS;
}
//----------------------------------------------------------------------------------------
GraphicsRenderer::~GraphicsRenderer() {
   // Empty
}

//----------------------------------------------------------------------------------------
void GraphicsRenderer::setupCamera() {
    camera->setFieldOfViewY(degreesToRadians(45));
    camera->setAspectRatio(1.0f);
    camera->setNearZDistance(0.1f);
    camera->setFarZDistance(100.0f);
    camera->setPosition(2.0, 1.5, 3.0);
    camera->lookAt(0,0,0);
}

//----------------------------------------------------------------------------------------
void GraphicsRenderer::setupShaders() {
    shaderProgram_LineRender.loadFromFile("data/shaders/LineRender.vs",
                                         "data/shaders/LineRender.fs");
}

//----------------------------------------------------------------------------------------
void GraphicsRenderer::setupVao() {
    glGenVertexArrays(1, &boundingVolume_vao);
    glBindVertexArray(boundingVolume_vao);

    // Enable Vertex Position Attribute Array
    glEnableVertexAttribArray(position_attribIndex);

    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GraphicsRenderer::setupBoundingCubeVertexData() {

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
    glBindVertexArray(boundingVolume_vao);

    // Upload Vertex Position Data:
    glGenBuffers(1, &boundingVolume_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, boundingVolume_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Upload Index Data:
    glGenBuffers(1, &boundingVolume_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boundingVolume_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    //-- Vertex Position Attribute Mapping:
    int32 elementsPerVertex = 3;
    int32 stride = 0;
    int32 offsetToFirstElement = 0;
    glVertexAttribPointer(position_attribIndex, elementsPerVertex,
            GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GraphicsRenderer::setupShaderUniforms() {
    shaderProgram_LineRender.setUniform("ModelViewMatrix", camera->getViewMatrix());
    shaderProgram_LineRender.setUniform("ProjectionMatrix", camera->getProjectionMatrix());
    shaderProgram_LineRender.setUniform("u_LineColor", vec4(0.4, 0.4, 0.4, 1.0));
}

//----------------------------------------------------------------------------------------
void GraphicsRenderer::updateShaderUniforms() {
    // Scale bounding box
    mat4 modelMatrix = glm::scale(mat4(), vec3(1.5,2.5,1.5));
    shaderProgram_LineRender.setUniform("ModelViewMatrix", camera->getViewMatrix()*modelMatrix);
}

//----------------------------------------------------------------------------------------
void GraphicsRenderer::draw() {
    updateShaderUniforms();

    glBindVertexArray(boundingVolume_vao);

    shaderProgram_LineRender.enable();
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
    shaderProgram_LineRender.disable();

    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void GraphicsRenderer::cleanup() {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &boundingVolume_vertexBuffer);
    glDeleteBuffers(1, &boundingVolume_indexBuffer);
    glDeleteVertexArrays(1, &boundingVolume_vao);

    CHECK_GL_ERRORS;
}
