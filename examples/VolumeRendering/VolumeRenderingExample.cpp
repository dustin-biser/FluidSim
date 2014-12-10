#include "VolumeRenderingExample.hpp"

using std::shared_ptr;

//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> demo =  VolumeRenderingExample::getInstance();
    demo->create(kScreenWidth, kScreenHeight, "Volume Rendering Demo", 1/80.0f);

    return 0;
}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> VolumeRenderingExample::getInstance() {
    static GlfwOpenGlWindow * instance = new VolumeRenderingExample();
    if (p_instance == nullptr) {
        p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//---------------------------------------------------------------------------------------
VolumeRenderingExample::~VolumeRenderingExample() {
    delete volumeRenderer;
}


//---------------------------------------------------------------------------------------
void VolumeRenderingExample::init() {
    volumeRenderer = new VolumeRenderer(kBoundingVolumeWidth,
                                        kBoundingVolumeHeight,
                                        kBoundingVolumeDepth,
                                        defaultFramebufferWidth(),
                                        defaultFramebufferHeight());

    volumeRenderer->enableDrawBoundingVolumeEdges();
    setupDepthSettings();
    initCamera();
    createTextureStorage();
    fillVolumeDensityTexture();
}

//---------------------------------------------------------------------------------------
void VolumeRenderingExample::setupDepthSettings() {
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);     // Enable writing to depth buffer
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);
    glEnable(GL_DEPTH_CLAMP); // Prevent near/far plane clipping

    glClearDepth(1.0f);
}


//---------------------------------------------------------------------------------------
void VolumeRenderingExample::initCamera() {
    camera.setNearZDistance(0.1f);
    camera.setFarZDistance(100.0f);
    camera.setPosition(1.1, 1.1, 1.0);
    camera.lookAt(0, 0, 0);
}


//---------------------------------------------------------------------------------------
void VolumeRenderingExample::createTextureStorage() {
    glGenTextures(1, &volumeDensity_texture3d);
    glBindTexture(GL_TEXTURE_3D, volumeDensity_texture3d);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, kBoundingVolumeWidth, kBoundingVolumeHeight,
        kBoundingVolumeDepth, 0, GL_RED, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    // Default border color = (0,0,0,0).

    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderingExample::fillVolumeDensityTexture() {
    glBindTexture(GL_TEXTURE_3D, volumeDensity_texture3d);

    const int width = kBoundingVolumeWidth;
    const int height = kBoundingVolumeHeight;
    const int depth = kBoundingVolumeDepth;
    float32 * data = new float32[depth*height*width];

    // Form a wedge like shape that is non-isotropic:
    // From the base upward, each slice should become thinner.
    for(int k(0); k < kBoundingVolumeDepth; ++k) {
        for(int j(0); j < kBoundingVolumeHeight; ++j) {
            for(int i(0); i < kBoundingVolumeWidth; ++i) {

                data[k*(height*width) + j*width + i] = 0.5f;
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
void VolumeRenderingExample::logic() {

}

//---------------------------------------------------------------------------------------
void VolumeRenderingExample::draw() {
    volumeRenderer->draw(camera, kRayStepSize, volumeDensity_texture3d);
}

//---------------------------------------------------------------------------------------
void VolumeRenderingExample::cleanup() {

}

