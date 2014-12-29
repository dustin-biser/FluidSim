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

const int kBoundingVolumeWidth  = 8;
const int kBoundingVolumeHeight = 8;
const int kBoundingVolumeDepth  = 8;

class MarchingCubesExample : public GlfwOpenGlWindow {

public:
    ~MarchingCubesExample();

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    MarchingCubesExample() = default; // Singleton. Prevent direct construction.

    MarchingCubesRenderer * marchingCubesRenderer;

    GLuint volumeDensity_texture3d;  // 3D data set to be rendered.


    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void cleanup();

    void createTextureStorage();
    void fillVolumeDensityTexture();

};
