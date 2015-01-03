/**
* @brief MarchineCubesExample
*
*/

#pragma once

#include <GlfwOpenGlWindow.hpp>

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

// Forward Declaration:
class MarchingCubesRenderer;


const int kScreenWidth = 1024;
const int kScreenHeight = 768;

const int kGridWidth = 4;
const int kGridHeight = 4;
const int kGridDepth = 4;

const float kIsoSurfaceThreshold = 1.0f;

class MarchingCubesExample : public GlfwOpenGlWindow {

public:
    ~MarchingCubesExample();

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    MarchingCubesExample() = default; // Singleton. Prevent direct construction.

    MarchingCubesRenderer * marchingCubesRenderer;

    //-- 3D data sets to be rendered:
    GLuint volumeDensity_texture3d;
    GLuint cubeDensity_texture3d;


    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void cleanup();

    void createTextureStorage();
    void fillVolumeDensityTexture();
    void fillCubeDensityTexture();

};
