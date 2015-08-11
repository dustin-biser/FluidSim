/**
* @brief VolumeRenderingExample
*
* @author Dustin Biser
*/
#include <GlfwOpenGlWindow.hpp>

#include <Synergy/Synergy.hpp>
using namespace Synergy;

#include "VolumeRenderer.hpp"

const int kScreenWidth = 1024;
const int kScreenHeight = 768;

const float kRayStepSize = 0.03;

class VolumeRenderingExample : public GlfwOpenGlWindow {

public:
    ~VolumeRenderingExample();

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    VolumeRenderingExample() = default; // Singleton. Prevent direct construction.

    VolumeRenderer * volumeRenderer;

	// 3D data set to be rendered.
    Texture3D volumeData;

	// Model transform for positioning volume data within scene.
	mat4 transform;

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void cleanup();

    void setupDepthSettings();
    void initCamera();
    void createTextureStorage();
    void fillVolumeDensityTexture();

};
