/**
* @brief SphGraphics.hpp
*
* @author Dustin Biser
*/

#pragma once

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include <vector>
using std::vector;

struct Particle;

class SphGraphics {
public:
    void init(const vector<Particle> & particles);
    void draw();

    void SetupShaders();
    void SetupUniforms();
    void SetupCamera();
    void SetupVboData(const vector<Particle> & particles);
    void UpdateGL(const vector<Particle> & particles);

private:
    ShaderProgram shaderProgram;
    Camera camera;
    GLuint vao;
    GLuint vbo_particlePositions;
    GLuint vbo_circle_vertices;
};