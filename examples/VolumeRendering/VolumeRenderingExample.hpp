/**
* @brief VolumeRenderingExample
*
* @author Dustin Biser
*/
#include <GlfwOpenGlWindow.hpp>

#include <Rigid3D/Rigid3D.hpp>
using namespace Rigid3D;

#include "VolumeRenderer.hpp"

const int kScreenWidth = 1024;
const int kScreenHeight = 768;

const int kBoundingVolumeWidth  = 512;
const int kBoundingVolumeHeight = 512;
const int kBoundingVolumeDepth  = 512;

const float kRayStepSize = 0.05;

class VolumeRenderingExample : public GlfwOpenGlWindow {

public:
    ~VolumeRenderingExample();

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    VolumeRenderingExample() = default; // Singleton. Prevent direct construction.

    VolumeRenderer * volumeRenderer;

    GLuint volumeDensity_texture3d;  // 3D data set to be rendered.

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void cleanup();

    void setupDepthSettings();
    void initCamera();
    void createTextureStorage();
    void fillVolumeDensityTexture();

};
