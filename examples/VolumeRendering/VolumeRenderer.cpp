#include "VolumeRenderer.hpp"

#include <cfloat>
using namespace std;


//---------------------------------------------------------------------------------------
VolumeRenderer::VolumeRenderer(
        uint32 boundingVolumeWidth,
        uint32 boundingVolumeHeight,
        uint32 boundingVolumeDepth,
        uint32 framebufferWidth,
        uint32 framebufferHeight)
    :
        boundingVolumeWidth(boundingVolumeWidth),
        boundingVolumeHeight(boundingVolumeHeight),
        boundingVolumeDepth(boundingVolumeDepth),
        framebufferWidth(framebufferWidth),
        framebufferHeight(framebufferHeight)
{
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

    glViewport(0, 0, framebufferWidth, framebufferHeight);

    createDepthBufferStorage();

    setupScreenQuadVboData();

    setupBoundingCubeVertexData();

    setupShaders();

    createTextureStorage();

    generateNoiseTexture();

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
VolumeRenderer::~VolumeRenderer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &framebuffer);
    glDeleteRenderbuffers(1, &depth_rbo);

    glDeleteTextures(1, &bvEntrance_texture2d);
    glDeleteTextures(1, &rayDirection_texture2d);
    glDeleteTextures(1, &accumulatedDensity_texture2d);
    glDeleteTextures(1, &noise_texture2d);

    glDeleteVertexArrays(1, &bvVao);
    glDeleteBuffers(1, &bvVertexBuffer);
    glDeleteBuffers(1, &bvIndexBuffer);

    glDeleteVertexArrays(1, &screenQuadVao);
    glDeleteBuffers(1, &screenQuadVertBuffer);
    glDeleteBuffers(1, &screenQuadIndexBuffer);

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::createTextureStorage() {

    //-- bvEntrance_texture2d:
    {
        glGenTextures(1, &bvEntrance_texture2d);
        glBindTexture(GL_TEXTURE_2D, bvEntrance_texture2d);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferWidth,
                framebufferHeight, 0, GL_RGB, GL_FLOAT, NULL);

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

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, framebufferWidth,
                framebufferHeight, 0, GL_RGBA, GL_FLOAT, NULL);

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

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, framebufferWidth,
                framebufferHeight, 0, GL_RED, GL_FLOAT, NULL);

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

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, framebufferWidth,
                framebufferHeight, 0, GL_RED, GL_FLOAT, NULL);

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
            framebufferWidth, framebufferHeight);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    CHECK_GL_ERRORS;
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
void VolumeRenderer::updateShaderUniforms(const Camera & camera) {
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
    bindFramebufferWithAttachments(framebuffer, bvEntrance_texture2d, depth_rbo);

    // Clear attached buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render only the front face of geometry.
    glCullFace(GL_BACK);

    renderBoundingVolume(shaderProgram_BvEntry);

    //-- Reset Defaults:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::composeRayDirectionTexture() {
    bindFramebufferWithAttachments(framebuffer, rayDirection_texture2d, depth_rbo);

    // Clear attached buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render only the back face of geometry.
    glCullFace(GL_FRONT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bvEntrance_texture2d);
    shaderProgram_RayDirection.setUniform("rayEntryTexture", 0);
    shaderProgram_RayDirection.setUniform("one_over_framebufferWidth",
            1.0f / float(framebufferWidth));
    shaderProgram_RayDirection.setUniform("one_over_framebufferHeight",
            1.0f / float(framebufferHeight));

    renderBoundingVolume(shaderProgram_RayDirection);

    //-- Reset Defaults:
    glBindTexture(GL_TEXTURE_2D, 0);
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
            1.0f / float(framebufferWidth));
    shaderProgram_RayMarch.setUniform("one_over_framebufferHeight",
            1.0f / float(framebufferHeight));

    // Render only front faces of bounding volume.
    glCullFace(GL_BACK);

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
void VolumeRenderer::accqiurePreviousGLSetings() {
    glGetFloatv(GL_DEPTH_CLEAR_VALUE, &prev_depth_clear_value);
    glGetFloatv(GL_COLOR_CLEAR_VALUE, prev_color_clear_value);
    glGetIntegerv(GL_CULL_FACE, &prev_cull_face);
    glGetIntegerv(GL_CULL_FACE_MODE, &prev_cull_face_mode);
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::restorePreviousGLSettings() {
    glClearDepth(prev_depth_clear_value);
    glClearColor(prev_color_clear_value[0],
            prev_color_clear_value[1],
            prev_color_clear_value[2],
            prev_color_clear_value[3]);
    if (prev_cull_face == GL_TRUE) {
        glEnable(GL_CULL_FACE);
    }
    glCullFace(prev_cull_face_mode);

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::draw(
        const Camera & camera,
        float32 rayStepSize,
        GLuint volumeData_texture3d
){
    accqiurePreviousGLSetings();

    updateShaderUniforms(camera);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    composeVolumeEntranceTexture();
    composeRayDirectionTexture();

    renderVolume(volumeData_texture3d, rayStepSize);


    restorePreviousGLSettings();
}
