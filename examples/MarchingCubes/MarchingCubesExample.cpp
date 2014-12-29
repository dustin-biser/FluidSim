#include "MarchingCubesExample.hpp"
#include "MarchingCubesRenderer.hpp"

using std::shared_ptr;

//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> demo =  MarchingCubesExample::getInstance();
    demo->create(kScreenWidth, kScreenHeight, "Volume Rendering Demo", 1/80.0f);

    return 0;
}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> MarchingCubesExample::getInstance() {
    static GlfwOpenGlWindow * instance = new MarchingCubesExample();
    if (p_instance == nullptr) {
        p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//---------------------------------------------------------------------------------------
MarchingCubesExample::~MarchingCubesExample() {
    delete marchingCubesRenderer;
}


//---------------------------------------------------------------------------------------
void MarchingCubesExample::init() {
    marchingCubesRenderer =
            new MarchingCubesRenderer(kBoundingVolumeWidth,
					                  kBoundingVolumeHeight,
                                      kBoundingVolumeDepth);

    createTextureStorage();
    fillVolumeDensityTexture();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glDisable(GL_DEPTH_TEST);

    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void MarchingCubesExample::createTextureStorage() {
    // volumeDensity_texture3d
    {
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
}

//---------------------------------------------------------------------------------------
void MarchingCubesExample::fillVolumeDensityTexture() {
    glBindTexture(GL_TEXTURE_3D, volumeDensity_texture3d);

    const int width = kBoundingVolumeWidth;
    const int height = kBoundingVolumeHeight;
    const int depth = kBoundingVolumeDepth;
    float32 * data = new float32[depth*height*width];

    for(int k(0); k < kBoundingVolumeDepth; ++k) {
        for(int j(0); j < kBoundingVolumeHeight; ++j) {
            for(int i(0); i < kBoundingVolumeWidth; ++i) {

                data[(k * height * width) + (j * width) + i] = i + j + k;

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
void MarchingCubesExample::logic() {

}

//---------------------------------------------------------------------------------------
void MarchingCubesExample::draw() {
    marchingCubesRenderer->render(camera, volumeDensity_texture3d);
}

//---------------------------------------------------------------------------------------
void MarchingCubesExample::cleanup() {

}
