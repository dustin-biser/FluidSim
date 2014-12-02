#include "VolumeRenderer.hpp"

using namespace std;

//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> volumeRenderDemo = VolumeRenderer::getInstance();
    volumeRenderDemo->create(kScreenWidth,
                             kScreenHeight,
                             "Volume Rendering Demo",
                             1/80.0f);

    return 0;
}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> VolumeRenderer::getInstance() {
    static GlfwOpenGlWindow * instance = new VolumeRenderer();
    if (p_instance == nullptr) {
        p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::setupGl() {
    // Render only the front face of geometry.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Setup depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    glEnable(GL_DEPTH_CLAMP); // Prevent near/far plane clipping

    glClearDepth(1.0f);
    glClearColor(0.3, 0.5, 0.7, 1.0);

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::init() {

    glGenVertexArrays(1, &bvVao);
    glBindVertexArray(bvVao);
        glEnableVertexAttribArray(position_attrib_index);
        glEnableVertexAttribArray(color_attrib_index);
    glBindVertexArray(0);

    setupBoundingCubeVertexData();
    setupCamera();
    initShaders();
    updateShaderUniforms();
    createTextureStorage();
    initCubeDensityTexture();

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::createTextureStorage() {
    glGenTextures(1, &volumeDensity_texture3d);
    glBindTexture(GL_TEXTURE_3D, volumeDensity_texture3d);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, kBoundingVolumeWidth, kBoundingVolumeHeight,
            kBoundingVolumeDepth, 0, GL_RED, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);


    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::initCubeDensityTexture() {
    glBindTexture(GL_TEXTURE_3D, volumeDensity_texture3d);

    const int width = kBoundingVolumeWidth;
    const int height = kBoundingVolumeHeight;
    const int depth = kBoundingVolumeDepth;
    float32 * data = new float32[depth*height*width];

    for(int k(100); k < kBoundingVolumeDepth-100; ++k) {
        for(int j(100); j < kBoundingVolumeHeight-100; ++j) {
            for(int i(100); i < kBoundingVolumeWidth-100; ++i) {
               data[k*(height*width) + j*width + i] = 0.01f;
            }
        }
    }

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kBoundingVolumeWidth,
            kBoundingVolumeHeight, kBoundingVolumeDepth, GL_RED, GL_FLOAT, data);


    delete [] data;
    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::setupCamera() {
    camera.setFieldOfViewY(45.0f);
    camera.setAspectRatio(1.0f);
    camera.setNearZDistance(0.1f);
    camera.setFarZDistance(100.0f);
//    camera.setPosition(1.0, 1.5, 2.5);
    camera.setPosition(1.0, 2.0, 3.0);
    camera.lookAt(0, 0, 0);
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::initShaders() {
    shaderProgram_BvEntry.loadFromFile("examples/VolumeRendering/shaders/BvEntry.vs",
                                       "examples/VolumeRendering/shaders/BvEntry.fs");
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::updateShaderUniforms() {
    shaderProgram_BvEntry.setUniform("modelView_matrix", camera.getViewMatrix());
    shaderProgram_BvEntry.setUniform("projection_matrix", camera.getProjectionMatrix());
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::setupBoundingCubeVertexData() {

    // Positions and Color-Coordinates.
    // Colors correspond to textureCoordinates into the 3D data set.
    float32 boundingVolumeVertices[] = {
            // Positions (x,y,z)   Color-Coordinate(s,t,r)
            -0.5,-0.5, 0.5,         0,1,0, // 0
             0.5,-0.5, 0.5,         1,1,0, // 1
             0.5, 0.5, 0.5,         1,1,1, // 2
            -0.5, 0.5, 0.5,         0,1,1, // 3
             0.5,-0.5, -0.5,        1,0,0, // 4
            -0.5,-0.5, -0.5,        0,0,0, // 5
            -0.5, 0.5, -0.5,        0,0,1, // 6
             0.5, 0.5, -0.5,        1,0,1  // 7
    };

    // Cube indices for GL_TRIANGLES
    GLushort indices [] = {
        0,1,2, 0,2,3,   // Front Face
        4,5,7, 7,5,6,   // Back Face
        5,0,3, 3,6,5,   // Left Face
        2,1,4, 4,7,2,   // Right Face
        6,3,2, 2,7,6,   // Top Face
        5,4,1, 1,0,5    // Bottom Face
    };

    // Store vertexAttribute mappings, and bound element buffer
    glBindVertexArray(bvVao);

    // Upload Vertex Position Data:
    glGenBuffers(1, &bvVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, bvVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(boundingVolumeVertices), boundingVolumeVertices, GL_STATIC_DRAW);

    // Upload Index Data:
    glGenBuffers(1, &bvIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bvIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    //-- Vertex Position Attribute Mapping:
    int32 elementsPerVertex = 3;
    int32 stride = 6 * sizeof(float32);
    int32 offsetToFirstElement = 0;
    glVertexAttribPointer(position_attrib_index, elementsPerVertex,
            GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

    //-- Vertex Color Attribute Mapping:
    elementsPerVertex = 3;
    stride = 6 * sizeof(float32);
    offsetToFirstElement = 3 * sizeof(float32);
    glVertexAttribPointer(color_attrib_index, elementsPerVertex,
            GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::computeVolumeEntryPoint() {
    glBindVertexArray(bvVao);

    shaderProgram_BvEntry.enable();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);
    shaderProgram_BvEntry.disable();


    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::logic() {
    updateShaderUniforms();
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::draw() {
    computeVolumeEntryPoint();

    // Cull Back Faces
    // Render front faces into the FrontFaceTexture

    // Cull Front Faces
    // Render back faces with FrontFaceTexture bound

}

//---------------------------------------------------------------------------------------
void VolumeRenderer::cleanup() {

}
