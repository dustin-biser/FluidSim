/**
* @brief SmokeGraphicsGPU.hpp
*
* @author Dustin Biser
*/

#pragma once

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include <vector>
using std::vector;

//----------------------------------------------------------------------------------------
// Graphics Parameters
//----------------------------------------------------------------------------------------
const int kScreenWidth = 800;
const int kScreenHeight = 800;
const int kViewWidth = 10;
const int kViewHeight = 10;

class SmokeGraphicsGPU {
public:
    void init();
    void draw();

    void SetupShaders();
    void SetupUniforms();
    void SetupCamera();
    void SetupVboData();
    void UpdateGL();

private:
    ShaderProgram shaderProgram;
    Camera camera;
    GLuint vao;
    GLuint vbo;
};