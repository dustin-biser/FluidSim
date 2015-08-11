#include "VolumeRenderingExample.hpp"

#include <glm/gtc/matrix_transform.hpp>

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
    volumeRenderer = new VolumeRenderer(
		    defaultFramebufferWidth(),
		    defaultFramebufferHeight()
    );

	volumeRenderer->enableBoundingVolumeEdges();

    setupDepthSettings();

    initCamera();

    createTextureStorage();

    fillVolumeDensityTexture();

	// Model transform for positioning volume-data with in the scene.
	transform = glm::translate(mat4(), vec3(0.0f, 0.0f, -1.5f));
	transform = glm::rotate(transform, -0.3f, vec3(0.0f,1.0,0.0f));
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
    camera.setPosition(0.0, 1.0, 1.0);
    camera.lookAt(0, 0, -1.5f);
}

//---------------------------------------------------------------------------------------
void VolumeRenderingExample::createTextureStorage() {
	TextureSpec textureSpec;
	textureSpec.width = 512;
	textureSpec.height = 512;
	textureSpec.depth = 512;
	textureSpec.internalFormat = GL_RED;
	textureSpec.format = GL_RED;
	textureSpec.dataType = GL_FLOAT;

	volumeData.allocateStorage(textureSpec);

    glBindTexture(GL_TEXTURE_3D, volumeData.name());
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

    const int width = volumeData.width();
    const int height = volumeData.height();
    const int depth = volumeData.depth();
    float32 * data = new float32[depth*height*width];

    // Form a wedge like shape that is non-isotropic:
    // From the base upward, each slice should become thinner.
    for(int k(0); k < depth; ++k) {
        for(int j(0); j < height; ++j) {
            for(int i(0); i < width; ++i) {

                data[k*(height*width) + j*width + i] = 0.2f * float(k)/depth;
            }
        }
    }

	glBindTexture(GL_TEXTURE_3D, volumeData.name());
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, width, height, depth,
            volumeData.format(), GL_FLOAT, data);


    delete [] data;
    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void VolumeRenderingExample::logic() {

}

//---------------------------------------------------------------------------------------
void VolumeRenderingExample::draw() {
	volumeRenderer->render (
			camera,
			kRayStepSize,
			volumeData,
			transform
	);
}

//---------------------------------------------------------------------------------------
void VolumeRenderingExample::cleanup() {

}

