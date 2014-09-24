#include "SphGraphics.hpp"
#include "SphSim.hpp"

#include <glm/gtc/type_ptr.hpp>

//----------------------------------------------------------------------------------------
void SphGraphics::init(const vector<Particle> & particles) {
    SetupShaders();
    SetupCamera();
    SetupVboData(particles);
    SetupUniforms();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SphGraphics::SetupShaders() {
    shaderProgram.loadFromFile("data/shaders/InstancedCircles.vs",
                               "data/shaders/InstancedCircles.fs");
}

//----------------------------------------------------------------------------------------
void SphGraphics::SetupCamera() {
    // Orthographic Projection
    camera = Camera(-kViewWidth*0.5f, kViewWidth*0.5f,    // left, right
                    -kViewHeight*0.5f, kViewHeight*0.5f,  // bottom, top
                    -1.0f, 1.0f);                         // zNear, zFar

    camera.setPosition(kViewWidth*0.5f, kViewHeight*0.5f, 0.0f);
}

//----------------------------------------------------------------------------------------
void SphGraphics::SetupVboData(const vector<Particle> & particles) {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Enable vertex attribute location 0, for circle vertices.
    glEnableVertexAttribArray(0);

    // Enable vertex attribute location 1, for particle positions.
    glEnableVertexAttribArray(1);

    //-- Setup VBO for particle position data:
    {
        // Copy position data to OpenGL buffer.
        glGenBuffers(1, &vbo_particlePositions);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_particlePositions);

        // TODO Dustin - Try extracting position data from all Particles and tightly
        // packing it into the VBO.  Check for performance increase.  May have
        // better GPU cache hit rate.

        GLuint particlePositionIndex = 1;
        GLsizeiptr particlePositionBytes = sizeof(Particle) * kNumParticles;
        const GLvoid * data = glm::value_ptr(particles[0].position);
        glBufferData(GL_ARRAY_BUFFER, particlePositionBytes, data, GL_STREAM_DRAW);

        GLsizei stride = sizeof(Particle);
        glVertexAttribPointer(particlePositionIndex, 2, GL_FLOAT, GL_FALSE, stride, 0);

        // Advance position attribute data once per instance.
        glVertexAttribDivisor(particlePositionIndex, 1);
    }

    //-- Vertex data for circle
    float32 sqrt2_over_2 = 0.70710678f;
    vector<vec2> circle_vertices = {
        vec2(0.0f, 0.0f),     // Center
        vec2(1.0f, 0.0f),     // 0 degrees
        vec2(1.0f, 1.0f),     // 45 degrees
        vec2(0.0f, 1.0f),     // 90 degrees
        vec2(-1.0f, 1.0f),    // 135 degrees
        vec2(-1.0f, 0.0f),    // 180 degrees
        vec2(-1.0f, -1.0f),   // 225 degrees
        vec2(0.0f, -1.0f),    // 270 degrees
        vec2(1.0f, -1.0f),    // 315 degrees
        vec2(1.0f, 0.0f),     // 360 degrees  [duplicate for triangle fan]
    };

    circle_vertices[2] *= sqrt2_over_2;
    circle_vertices[4] *= sqrt2_over_2;
    circle_vertices[6] *= sqrt2_over_2;
    circle_vertices[8] *= sqrt2_over_2;

    // Scale vertex data
    for(vec2 &v : circle_vertices) { v *= kParticleRadius; }

    //-- Setup VBO for circle vertices:
    {
        // Non-instanced data.

        // Copy position data to OpenGL buffer.
        glGenBuffers(1, &vbo_circle_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_circle_vertices);

        GLuint vertexPositionIndex = 0;
        GLsizeiptr vertexPositionBytes = sizeof(vec2) * circle_vertices.size();
        glBufferData(GL_ARRAY_BUFFER, vertexPositionBytes, &(circle_vertices.data()[0]),
                     GL_STATIC_DRAW);
        glVertexAttribPointer(vertexPositionIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SphGraphics::SetupUniforms() {
    shaderProgram.setUniform("u_Color", vec3(0.2f, 0.2f, 0.6f));

    mat4 view = camera.getViewMatrix();
    mat4 projection = camera.getProjectionMatrix();
    shaderProgram.setUniform("u_ViewProjectionMatrix", projection * view);
}

//----------------------------------------------------------------------------------------
void SphGraphics::UpdateGL(const vector<Particle> & particles) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo_particlePositions);
    const GLvoid * data = glm::value_ptr(particles[0].position);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Particle)*kNumParticles, data);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SphGraphics::draw() {
    shaderProgram.enable();
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 10,  GLsizei(kNumParticles));
    shaderProgram.disable();

    CHECK_GL_ERRORS;
}
