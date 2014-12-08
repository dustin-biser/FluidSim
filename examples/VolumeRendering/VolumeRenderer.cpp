#include "VolumeRenderer.hpp"

#include <cfloat>
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
    glGenFramebuffers(1, &framebuffer);

    glGenVertexArrays(1, &bvVao);
    glBindVertexArray(bvVao);
        glEnableVertexAttribArray(position_attrib_index);
        glEnableVertexAttribArray(color_attrib_index);
    glBindVertexArray(0);

    glGenVertexArrays(1, &screenQuadVao);
    glBindVertexArray(screenQuadVao);
        glEnableVertexAttribArray(position_attrib_index);
        glEnableVertexAttribArray(textureCoord_attrib_index);
    glBindVertexArray(0);

    glViewport(0, 0, defaultFramebufferWidth(), defaultFramebufferHeight());

    createDepthBufferStorage();

    setupScreenQuadVboData();

    setupBoundingCubeVertexData();

    initCamera();

    setupShaders();

    updateShaderUniforms();

    createTextureStorage();

    fillCubeDensityTexture();

    generateNoiseTexture();

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::createTextureStorage() {

    //-- volumeDensity_texture3d:
    {
        glGenTextures(1, &volumeDensity_texture3d);
        glBindTexture(GL_TEXTURE_3D, volumeDensity_texture3d);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, kBoundingVolumeWidth, kBoundingVolumeHeight,
                kBoundingVolumeDepth, 0, GL_RED, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        // Default border color = (0,0,0,0).

        glBindTexture(GL_TEXTURE_3D, 0);
        CHECK_GL_ERRORS;
    }

    //-- bvEntrace_texture2d:
    {
        glGenTextures(1, &bvEntrace_texture2d);
        glBindTexture(GL_TEXTURE_2D, bvEntrace_texture2d);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, defaultFramebufferWidth(),
                defaultFramebufferHeight(), 0, GL_RGB, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_GL_ERRORS;
    }

    //-- rayDirection_texture2d:
    {
        glGenTextures(1, &rayDirection_texture2d);
        glBindTexture(GL_TEXTURE_2D, rayDirection_texture2d);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, defaultFramebufferWidth(),
                defaultFramebufferHeight(), 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_GL_ERRORS;
    }

    //-- accumulatedDensity_texture2d:
    {
        glGenTextures(1, &accumulatedDensity_texture2d);
        glBindTexture(GL_TEXTURE_2D, accumulatedDensity_texture2d);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, defaultFramebufferWidth(),
                defaultFramebufferHeight(), 0, GL_RED, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_GL_ERRORS;
    }

    //-- noise_texture2d:
    {
        glGenTextures(1, &noise_texture2d);
        glBindTexture(GL_TEXTURE_2D, noise_texture2d);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, defaultFramebufferWidth(),
                defaultFramebufferHeight(), 0, GL_RED, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_GL_ERRORS;
    }

}

//---------------------------------------------------------------------------------------
void VolumeRenderer::createDepthBufferStorage() {
    glGenRenderbuffers(1, &depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32,
            defaultFramebufferWidth(), defaultFramebufferHeight());

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::fillCubeDensityTexture() {
    glBindTexture(GL_TEXTURE_3D, volumeDensity_texture3d);

    const int width = kBoundingVolumeWidth;
    const int height = kBoundingVolumeHeight;
    const int depth = kBoundingVolumeDepth;
    float32 * data = new float32[depth*height*width];

    // Form a wedge like shape that is non-isotropic:
    // From the base upward, each slice should become thinner.
    for(int k(0); k < kBoundingVolumeDepth; ++k) {
        for(int j(0); j < kBoundingVolumeHeight; ++j) {
            for(int i(0); i < kBoundingVolumeWidth; ++i) {

               data[k*(height*width) + j*width + i] = 0.5f;
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
void VolumeRenderer::initCamera() {
    camera.setNearZDistance(0.1f);
    camera.setFarZDistance(100.0f);
    camera.setPosition(1.1, 1.1, 1.0);
    camera.lookAt(0, 0, 0);
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::setupShaders() {
    shaderProgram_BvEntry.loadFromFile(
            "examples/VolumeRendering/shaders/BvEntry.vs",
            "examples/VolumeRendering/shaders/BvEntry.fs");

    shaderProgram_RayDirection.loadFromFile(
            "examples/VolumeRendering/shaders/RayDirection.vs",
            "examples/VolumeRendering/shaders/RayDirection.fs");

    shaderProgram_RayMarch.loadFromFile(
            "examples/VolumeRendering/shaders/RayMarch.vs",
            "examples/VolumeRendering/shaders/RayMarch.fs");

    shaderProgram_RenderTexture.loadFromFile(
            "examples/VolumeRendering/shaders/ScreenQuad.vs",
            "examples/VolumeRendering/shaders/ScreenQuad.fs");

    shaderProgram_NoiseGenerator.loadFromFile(
            "examples/VolumeRendering/shaders/NoiseGenerator.vs",
            "examples/VolumeRendering/shaders/NoiseGenerator.fs");
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::updateShaderUniforms() {
    shaderProgram_BvEntry.setUniform("modelView_matrix", camera.getViewMatrix());
    shaderProgram_BvEntry.setUniform("projection_matrix", camera.getProjectionMatrix());

    shaderProgram_RayDirection.setUniform("modelView_matrix", camera.getViewMatrix());
    shaderProgram_RayDirection.setUniform("projection_matrix", camera.getProjectionMatrix());

    shaderProgram_RayMarch.setUniform("modelView_matrix", camera.getViewMatrix());
    shaderProgram_RayMarch.setUniform("projection_matrix", camera.getProjectionMatrix());
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

//----------------------------------------------------------------------------------------
void VolumeRenderer::setupScreenQuadVboData() {
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
        glVertexAttribPointer(position_attrib_index, elementsPerVertex, GL_FLOAT,
                GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

        //-- Texture Coordinate Data:
        elementsPerVertex = 2;
        stride = 4*sizeof(float32);
        offsetToFirstElement = 2*sizeof(float32);
        glVertexAttribPointer(textureCoord_attrib_index, elementsPerVertex, GL_FLOAT,
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
static void bindFramebufferWithAttachments(
        GLuint framebuffer,
        GLuint colorTextureName,
        GLuint depthRenderBufferObject)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //-- Attach color, depth, and stencil buffers to framebuffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            colorTextureName, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
            depthRenderBufferObject);

    CHECK_FRAMEBUFFER_COMPLETENESS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::renderBoundingVolume(const ShaderProgram & shader) {

    glBindVertexArray(bvVao);

    shader.enable();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);
    shader.disable();

    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::renderScreenQuad(const ShaderProgram & shader) {
    glBindVertexArray(screenQuadVao);

    shader.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    shader.disable();

    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::composeVolumeEntranceTexture() {
    bindFramebufferWithAttachments(framebuffer, bvEntrace_texture2d, depth_rbo);

    // Clear attached buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render only the front face of geometry.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    renderBoundingVolume(shaderProgram_BvEntry);

    //-- Reset Defaults:
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::composeRayDirectionTexture() {
    bindFramebufferWithAttachments(framebuffer, rayDirection_texture2d, depth_rbo);

    // Clear attached buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render only the back face of geometry.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bvEntrace_texture2d);
    shaderProgram_RayDirection.setUniform("rayEntryTexture", 0);
    shaderProgram_RayDirection.setUniform("framebufferWidth", defaultFramebufferWidth());
    shaderProgram_RayDirection.setUniform("framebufferHeight", defaultFramebufferHeight());

    renderBoundingVolume(shaderProgram_RayDirection);

    //-- Reset Defaults:
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::renderVolume(GLuint in_dataTexture3d, float stepSize)
{
    // Clear initial density texture values:
    bindFramebufferWithAttachments(framebuffer, accumulatedDensity_texture2d, depth_rbo);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDepthFunc(GL_LESS); // Only fragments with depth less than 1.0 will pass.
    glDepthRange(0.0f, 1.0f);
    glDepthMask(GL_FALSE); // Prevent writing to depth buffer.

    marchRaysForward(in_dataTexture3d, stepSize);

    renderTextureToScreen(accumulatedDensity_texture2d);

    //-- Restore defaults:
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::marchRaysForward(
        GLuint in_dataTexture3d,
        float stepSize)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, in_dataTexture3d);
    shaderProgram_RayMarch.setUniform("dataTexture3d", 0);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, rayDirection_texture2d);
    shaderProgram_RayMarch.setUniform("rayDirection_texture2d", 1);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, noise_texture2d);
    shaderProgram_RayMarch.setUniform("noise_texture2d", 2);

    shaderProgram_RayMarch.setUniform("stepSize", stepSize);

    shaderProgram_RayMarch.setUniform("one_over_framebufferWidth",
            1.0f / float(defaultFramebufferWidth()));
    shaderProgram_RayMarch.setUniform("one_over_framebufferHeight",
            1.0f / float(defaultFramebufferHeight()));

    renderBoundingVolume(shaderProgram_RayMarch);

    //-- Reset defaults:
    glBindTexture(GL_TEXTURE_3D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::renderTextureToScreen(GLuint textureName) {
    // Use default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureName);
    shaderProgram_RenderTexture.setUniform("u_textureUnit", 0);

    renderScreenQuad(shaderProgram_RenderTexture);

    //-- Restore defaults:
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::generateNoiseTexture() {
    bindFramebufferWithAttachments(framebuffer, noise_texture2d, depth_rbo);
    glClearDepth(1.0f);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScreenQuad(shaderProgram_NoiseGenerator);

    //-- Restore defaults:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::inspectTexture(GLuint textureName) {
    const int width = defaultFramebufferWidth();
    const int height = defaultFramebufferHeight();
    float * data = new float[width * height];

    glFinish();
    glBindTexture(GL_TEXTURE_2D, textureName);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);

    float min = FLT_MAX;
    float max = FLT_MIN;
    float value;
    for(int i(0); i < height; ++i) {
        for(int j(0); j < width; ++j) {
            value = data[i*width + j];
            if(value < min) {
                min = value;
            }
            if(value > max) {
                max = value;
            }
        }
    }

    cout << "min: " << min << endl;
    cout << "max: " << max << endl;

    delete [] data;
    glBindTexture(GL_TEXTURE_2D, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::logic() {
    updateShaderUniforms();
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::draw() {
    composeVolumeEntranceTexture();
    composeRayDirectionTexture();
    renderVolume(volumeDensity_texture3d, kRayStepSize);
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::cleanup() {

}
