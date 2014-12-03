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

    createDepthStencilBufferStorage();

    setupScreenQuadVboData();

    setupBoundingCubeVertexData();

    initCamera();

    setupShaders();

    updateShaderUniforms();

    createTextureStorage();

    initCubeDensityTexture();

    glViewport(0, 0, defaultFramebufferWidth(), defaultFramebufferHeight());

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

        glBindTexture(GL_TEXTURE_3D, 0);
        CHECK_GL_ERRORS;
    }

    //-- bvFrontFace_texture2d
    {
        glGenTextures(1, &bvFrontFace_texture2d);
        glBindTexture(GL_TEXTURE_2D, bvFrontFace_texture2d);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, defaultFramebufferWidth(),
                defaultFramebufferHeight(), 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
        CHECK_GL_ERRORS;
    }

}

//---------------------------------------------------------------------------------------
void VolumeRenderer::createDepthStencilBufferStorage() {
    glGenRenderbuffers(1, &depth_stencil_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
            defaultFramebufferWidth(), defaultFramebufferHeight());

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
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

    shaderProgram_RenderTexture.loadFromFile(
            "examples/VolumeRendering/shaders/ScreenQuad.vs",
            "examples/VolumeRendering/shaders/ScreenQuad.fs");
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

//---------------------------------------------------------------------------------------
void VolumeRenderer::renderBoundingVolume() {
    glBindVertexArray(bvVao);

    shaderProgram_BvEntry.enable();
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);
    shaderProgram_BvEntry.disable();

    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::computeVolumeEntryPoint() {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    //-- Attach color, depth, and stencil buffers to framebuffer.
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
            bvFrontFace_texture2d, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
            depth_stencil_rbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
            depth_stencil_rbo);
    CHECK_FRAMEBUFFER_COMPLETENESS;

    //-- Stencil only screen fragments covered by bounding volume:
    {
        // Clear attached buffers
        glClearStencil(0);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);

        glStencilMask(0xFF);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);

        glEnable(GL_STENCIL_TEST);

            renderBoundingVolume();

        glDisable(GL_STENCIL_TEST);
    }

    //-- Reset Defaults:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderer::renderTextureToScreen(GLuint textureName) {
    // Use default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureName);
    shaderProgram_RenderTexture.setUniform("u_textureUnit", 0);

    glBindVertexArray(screenQuadVao);

    shaderProgram_RenderTexture.enable();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL);
    shaderProgram_RenderTexture.disable();


    glBindTexture(GL_TEXTURE_2D, 0);
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
    renderTextureToScreen(bvFrontFace_texture2d);


    // Cull Back Faces
    // Render front faces into the FrontFaceTexture

    // Cull Front Faces
    // Render back faces with FrontFaceTexture bound

}

//---------------------------------------------------------------------------------------
void VolumeRenderer::cleanup() {

}
