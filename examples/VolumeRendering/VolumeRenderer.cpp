#include "VolumeRenderer.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

//---------------------------------------------------------------------------------------
VolumeRenderer::VolumeRenderer (
        uint32 framebufferWidth,
        uint32 framebufferHeight
) :
        framebufferWidth(framebufferWidth),
        framebufferHeight(framebufferHeight),
        edgeDrawingEnabled(true)
{
    glGenFramebuffers(1, &framebuffer);

    glGenVertexArrays(1, &bvVao);
    glBindVertexArray(bvVao);
    glEnableVertexAttribArray(uvCoord_attrib_index);
    glEnableVertexAttribArray(color_attrib_index);
    glBindVertexArray(0);

    glGenVertexArrays(1, &screenQuadVao);
    glBindVertexArray(screenQuadVao);
    glEnableVertexAttribArray(uvCoord_attrib_index);
    glEnableVertexAttribArray(zLayerCoord_attrib_index);
    glBindVertexArray(0);

    glGenVertexArrays(1, &bvEdgesVao);
    glBindVertexArray(bvEdgesVao);
    glEnableVertexAttribArray(uvCoord_attrib_index);
    glBindVertexArray(0);

    glViewport(0, 0, framebufferWidth, framebufferHeight);

    setupScreenQuadVboData();

    setupBoundingVolumeVertexData();

    setupBoundingVolumeEdgesVAO();

    setupShaders();

    createTextureStorage();

    generateNoiseTexture();

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
VolumeRenderer::~VolumeRenderer() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &framebuffer);

    glDeleteTextures(1, &bvEntrance_texture2d);
    glDeleteTextures(1, &rayDirection_texture2d);
    glDeleteTextures(1, &accumulatedDensity_texture2d);
    glDeleteTextures(1, &noise_texture2d);

    glDeleteVertexArrays(1, &bvVao);
    glDeleteBuffers(1, &bvVertexBuffer);
    glDeleteBuffers(1, &bvIndexBuffer);

    glDeleteVertexArrays(1, &screenQuadVao);
    glDeleteBuffers(1, &screenQuadVertexBuffer);
    glDeleteBuffers(1, &screenQuadIndexBuffer);

    glDeleteVertexArrays(1, &bvEdgesVao);
    glDeleteBuffers(1, &bvEdgesVertexBuffer);
    glDeleteBuffers(1, &bvEdgesIndexBuffer);

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
void VolumeRenderer::setupShaders() {
    shaderProgram_BvEntry.generateProgramObject();
    shaderProgram_BvEntry.attachVertexShader(
            "examples/VolumeRendering/shaders/BvEntry.vs");
    shaderProgram_BvEntry.attachFragmentShader(
            "examples/VolumeRendering/shaders/BvEntry.fs");
    shaderProgram_BvEntry.link();

    shaderProgram_RayDirection.generateProgramObject();
    shaderProgram_RayDirection.attachVertexShader(
            "examples/VolumeRendering/shaders/RayDirection.vs");
    shaderProgram_RayDirection.attachFragmentShader(
            "examples/VolumeRendering/shaders/RayDirection.fs");
    shaderProgram_RayDirection.link();

    shaderProgram_RayMarch.generateProgramObject();
    shaderProgram_RayMarch.attachVertexShader(
            "examples/VolumeRendering/shaders/RayMarch.vs");
    shaderProgram_RayMarch.attachFragmentShader(
            "examples/VolumeRendering/shaders/RayMarch.fs");
    shaderProgram_RayMarch.link();

    shaderProgram_RenderTexture.generateProgramObject();
    shaderProgram_RenderTexture.attachVertexShader(
            "examples/VolumeRendering/shaders/ScreenQuad.vs");
    shaderProgram_RenderTexture.attachFragmentShader(
            "examples/VolumeRendering/shaders/DensityTransferFunc.fs");
    shaderProgram_RenderTexture.link();

    shaderProgram_NoiseGenerator.generateProgramObject();
    shaderProgram_NoiseGenerator.attachVertexShader(
            "examples/VolumeRendering/shaders/NoiseGenerator.vs");
    shaderProgram_NoiseGenerator.attachFragmentShader(
            "examples/VolumeRendering/shaders/NoiseGenerator.fs");
    shaderProgram_NoiseGenerator.link();

    shaderProgram_LineRender.generateProgramObject();
    shaderProgram_LineRender.attachVertexShader(
            "examples/VolumeRendering/shaders/LineRender.vs");
    shaderProgram_LineRender.attachFragmentShader(
            "examples/VolumeRendering/shaders/LineRender.fs");
    shaderProgram_LineRender.link();
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::updateShaderUniforms (
		const Camera & camera,
		const mat4 & transform
) {
	mat4 modelView = camera.getViewMatrix() * transform;
	mat4 projMatrix = camera.getProjectionMatrix();
    shaderProgram_BvEntry.setUniform("modelView_matrix", modelView);
    shaderProgram_BvEntry.setUniform("projection_matrix", projMatrix);

    shaderProgram_RayDirection.setUniform("modelView_matrix", modelView);
    shaderProgram_RayDirection.setUniform("projection_matrix", projMatrix);

    shaderProgram_RayMarch.setUniform("modelView_matrix", modelView);
    shaderProgram_RayMarch.setUniform("projection_matrix", projMatrix);

    if (edgeDrawingEnabled) {
        shaderProgram_LineRender.setUniform("ModelViewMatrix", modelView);
        shaderProgram_LineRender.setUniform("ProjectionMatrix", projMatrix);
        shaderProgram_LineRender.setUniform("u_LineColor", vec4(0.4, 0.4, 0.4, 1.0));
    }
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::setupBoundingVolumeVertexData() {
    // Positions and Color-Coordinates.
    // Colors correspond to textureCoordinates into the 3D data set.
    float32 bvVertices[] = {
            // Positions (x,y,z)   Color-Coordinate(s,t,r)
            -0.5,-0.5, 0.5,         1,0,0, // 0
             0.5,-0.5, 0.5,         1,1,0, // 1
             0.5, 0.5, 0.5,         1,1,1, // 2
            -0.5, 0.5, 0.5,         1,0,1, // 3
             0.5,-0.5, -0.5,        0,1,0, // 4
            -0.5,-0.5, -0.5,        0,0,0, // 5
            -0.5, 0.5, -0.5,        0,0,1, // 6
             0.5, 0.5, -0.5,        0,1,1  // 7
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(bvVertices), bvVertices, GL_STATIC_DRAW);

    // Upload Index Data:
    glGenBuffers(1, &bvIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bvIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    //-- Vertex Position Attribute Mapping:
    int32 elementsPerVertex = 3;
    int32 stride = 6 * sizeof(float32);
    int32 offsetToFirstElement = 0;
    glVertexAttribPointer(uvCoord_attrib_index, elementsPerVertex,
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
void VolumeRenderer::setupBoundingVolumeEdgesVAO() {
    //  Bounding volume vertex offsets in model-space
    float32 bvVertices[] = {
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
    glBindVertexArray(bvEdgesVao);

    // Upload Vertex Position Data:
    glGenBuffers(1, &bvEdgesVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, bvEdgesVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bvVertices), bvVertices, GL_STATIC_DRAW);

    // Upload Index Data:
    glGenBuffers(1, &bvEdgesIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bvEdgesIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    //-- Vertex Position Attribute Mapping:
    int32 elementsPerVertex = 3;
    int32 stride = 0;
    int32 offsetToFirstElement = 0;
    glVertexAttribPointer(uvCoord_attrib_index, elementsPerVertex,
            GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void VolumeRenderer::setupScreenQuadVboData() {
    glBindVertexArray(screenQuadVao);

    glGenBuffers(1, &screenQuadVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVertexBuffer);

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
        glVertexAttribPointer(uvCoord_attrib_index, elementsPerVertex, GL_FLOAT,
                GL_FALSE, stride, reinterpret_cast<void *>(offsetToFirstElement));

        //-- Texture Coordinate Data:
        elementsPerVertex = 2;
        stride = 4*sizeof(float32);
        offsetToFirstElement = 2*sizeof(float32);
        glVertexAttribPointer(zLayerCoord_attrib_index, elementsPerVertex, GL_FLOAT,
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
void VolumeRenderer::enableBoundingVolumeEdges() {
    edgeDrawingEnabled = true;
}

//----------------------------------------------------------------------------------------
void VolumeRenderer::disableBoundingVolumeEdges() {
    edgeDrawingEnabled = false;
}

//----------------------------------------------------------------------------------------
static void bindFramebufferWithAttachments(
        GLuint framebuffer,
        GLuint colorTextureName)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            colorTextureName, 0);

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
    bindFramebufferWithAttachments(framebuffer, bvEntrance_texture2d);

    // Clear attached buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Render only the front face of geometry.
    glCullFace(GL_BACK);

    renderBoundingVolume(shaderProgram_BvEntry);

    //-- Reset Defaults:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::composeRayDirectionTexture() {
    bindFramebufferWithAttachments(framebuffer, rayDirection_texture2d);

    // Clear attached buffer
    glClear(GL_COLOR_BUFFER_BIT);

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
void VolumeRenderer::renderVolume (
		const Texture3D & volumeData,
		float stepSize
) {
    // Reuse bvEntrance_texture2d texture for accumulating density values along ray:
    accumulatedDensity_texture2d = bvEntrance_texture2d;

    // Clear initial density texture values:
    bindFramebufferWithAttachments(framebuffer, accumulatedDensity_texture2d);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

	rayMarch(volumeData, stepSize);

    renderTextureToScreen(accumulatedDensity_texture2d);

    CHECK_GL_ERRORS;
}
//---------------------------------------------------------------------------------------
void VolumeRenderer::renderBoundingVolumeEdges() {
    glBindVertexArray(bvEdgesVao);

    shaderProgram_LineRender.enable();
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, 0);
    shaderProgram_LineRender.disable();

    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::rayMarch(
		const Texture3D  & volumeData,
		float stepSize)
{
    glActiveTexture(GL_TEXTURE0);
	volumeData.bind();
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
    bindFramebufferWithAttachments(framebuffer, noise_texture2d);
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);

    renderScreenQuad(shaderProgram_NoiseGenerator);

    //-- Restore defaults:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::accqiurePreviousGLSetings() {
    glGetFloatv(GL_COLOR_CLEAR_VALUE, prev_color_clear_value);
    glGetIntegerv(GL_CULL_FACE, &prev_cull_face);
    glGetIntegerv(GL_CULL_FACE_MODE, &prev_cull_face_mode);
    glGetBooleanv(GL_DEPTH_TEST, &prev_depth_test_enabled);
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::restorePreviousGLSettings() {
    glClearColor (
		    prev_color_clear_value[0],
            prev_color_clear_value[1],
            prev_color_clear_value[2],
            prev_color_clear_value[3]
    );

    if (prev_cull_face == GL_TRUE) {
        glEnable(GL_CULL_FACE);
    }
    if (prev_depth_test_enabled == GL_TRUE) {
       glEnable(GL_DEPTH_TEST);
    }
    glCullFace(prev_cull_face_mode);

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::render (
		const Camera &camera,
		float32 rayStepSize,
		const Texture3D & volumeData,
		const mat4 & transform
) {
    accqiurePreviousGLSetings();

    updateShaderUniforms(camera, transform);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

    composeVolumeEntranceTexture();
    composeRayDirectionTexture();

    renderVolume(volumeData, rayStepSize);

    if(edgeDrawingEnabled) {
        glEnable(GL_DEPTH_TEST);
        renderBoundingVolumeEdges();
    }


    restorePreviousGLSettings();
}
