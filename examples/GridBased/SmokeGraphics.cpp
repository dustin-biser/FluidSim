#include "SmokeGraphics.hpp"
#include "SmokeSim.hpp"

#include <glm/gtc/type_ptr.hpp>

//----------------------------------------------------------------------------------------
void SmokeGraphics::init() {
    SetupShaders();
    SetupCamera();
    SetupVboData();
    SetupUniforms();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::SetupShaders() {
    shaderProgram.loadFromFile("data/shaders/InstancedCircles.vs",
            "data/shaders/InstancedCircles.fs");
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::SetupCamera() {
    // Orthographic Projection
    camera = Camera(-kViewWidth*0.5f, kViewWidth*0.5f,    // left, right
            -kViewHeight*0.5f, kViewHeight*0.5f,  // bottom, top
            -1.0f, 1.0f);                         // zNear, zFar

    // Place the camera at the center of scene, which is fine for orthographic projection.
    // (e.g. no mesh/camera clipping will occur)
    camera.setPosition(kViewWidth*0.5f, kViewHeight*0.5f, 0.0f);
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::SetupVboData() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glEnableVertexAttribArray(0);

    // TODO Dustin - Put data into vbo and then map that data to vertex locations using vao.


    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::SetupUniforms() {
    shaderProgram.setUniform("u_Color", vec3(0.2f, 0.2f, 0.6f));

    mat4 view = camera.getViewMatrix();
    mat4 projection = camera.getProjectionMatrix();
    shaderProgram.setUniform("u_ViewProjectionMatrix", projection * view);
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::UpdateGL() {

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SmokeGraphics::draw() {
    shaderProgram.enable();
//        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 10,  GLsizei(kNumParticles));
    shaderProgram.disable();

    CHECK_GL_ERRORS;
}
