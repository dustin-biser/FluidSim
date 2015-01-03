#include "MarchingCubesExample.hpp"
#include "MarchingCubesRenderer.hpp"

using std::shared_ptr;

//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> demo =  MarchingCubesExample::getInstance();
    demo->create(kScreenWidth, kScreenHeight, "Marching Cubes Demo", 1/80.0f);

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
            new MarchingCubesRenderer(kGridWidth, kGridHeight, kGridDepth);

    createTextureStorage();
    fillVolumeDensityTexture();
    fillCubeDensityTexture();

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

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, kGridWidth, kGridHeight,
                kGridDepth, 0, GL_RED, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        // Default border color = (0,0,0,0).

        glBindTexture(GL_TEXTURE_3D, 0);
        CHECK_GL_ERRORS;
    }

    // cubeDensity_texture3d
    {
        glGenTextures(1, &cubeDensity_texture3d);
        glBindTexture(GL_TEXTURE_3D, cubeDensity_texture3d);

        glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, kGridWidth, kGridHeight,
                kGridDepth, 0, GL_RED, GL_FLOAT, NULL);

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

    const int width = kGridWidth;
    const int height = kGridHeight;
    const int depth = kGridDepth;
    float32 * data = new float32[depth*height*width];

    for(int k(0); k < kGridDepth; ++k) {
        for(int j(0); j < kGridHeight; ++j) {
            for(int i(0); i < kGridWidth; ++i) {
                data[(k * height * width) + (j * width) + i] = 0;
            }
        }
    }

    // Initialize first voxel:
    data[(0 * height * width) + (0 * width) + 0] = 1;  // Vertex 0
    data[(0 * height * width) + (0 * width) + 1] = 2;  // Vertex 1
    data[(0 * height * width) + (1 * width) + 1] = 2;  // Vertex 2
    data[(0 * height * width) + (1 * width) + 0] = 2;  // Vertex 3

    data[(1 * height * width) + (0 * width) + 0] = 2;  // Vertex 4
    data[(1 * height * width) + (0 * width) + 1] = 2;  // Vertex 5
    data[(1 * height * width) + (1 * width) + 1] = 2;  // Vertex 6
    data[(1 * height * width) + (1 * width) + 0] = 2;  // Vertex 7

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kGridWidth,
            kGridHeight, kGridDepth, GL_RED, GL_FLOAT, data);


    delete [] data;
    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}


//---------------------------------------------------------------------------------------
void MarchingCubesExample::fillCubeDensityTexture() {
    glBindTexture(GL_TEXTURE_3D, volumeDensity_texture3d);

    float32 * data = new float32[kGridDepth * kGridHeight * kGridWidth];

    // Set all border values below isoSurfaceThreshold, and interior cells above
    // isoSurfaceThreshold.
    for(int k(0); k < kGridDepth; ++k) {
        for(int j(0); j < kGridHeight; ++j) {
            for(int i(0); i < kGridWidth; ++i) {
                float value = isoSurfaceThreshold + 1.0f;

                if (i == 0 || i == kGridWidth - 1)
                    value = isoSurfaceThreshold - 1.0f;
                if (j == 0 || j == kGridHeight - 1)
                    value = isoSurfaceThreshold - 1.0f;
                if (k == 0 || k == kGridDepth - 1)
                    value = isoSurfaceThreshold - 1.0f;

                data[(k * kGridHeight * kGridWidth) + (j * kGridWidth) + i] = value;
            }
        }
    }

    // Initialize first voxel:
    float value = isoSurfaceThreshold;
    data[(0 * kGridHeight * kGridWidth) + (0 * kGridWidth) + 0] = value - 1;  // Vertex 0
    data[(0 * kGridHeight * kGridWidth) + (0 * kGridWidth) + 1] = value + 1;  // Vertex 1
    data[(0 * kGridHeight * kGridWidth) + (1 * kGridWidth) + 1] = value + 1;  // Vertex 2
    data[(0 * kGridHeight * kGridWidth) + (1 * kGridWidth) + 0] = value + 1;  // Vertex 3

    data[(1 * kGridHeight * kGridWidth) + (0 * kGridWidth) + 0] = value + 1;  // Vertex 4
    data[(1 * kGridHeight * kGridWidth) + (0 * kGridWidth) + 1] = value + 1;  // Vertex 5
    data[(1 * kGridHeight * kGridWidth) + (1 * kGridWidth) + 1] = value + 1;  // Vertex 6
    data[(1 * kGridHeight * kGridWidth) + (1 * kGridWidth) + 0] = value + 1;  // Vertex 7

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kGridWidth,
            kGridHeight, kGridDepth, GL_RED, GL_FLOAT, data);


    delete [] data;
    glBindTexture(GL_TEXTURE_3D, 0);
    CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void MarchingCubesExample::logic() {

}

//---------------------------------------------------------------------------------------
void MarchingCubesExample::draw() {
    marchingCubesRenderer->render(camera, cubeDensity_texture3d, isoSurfaceThreshold);
}

//---------------------------------------------------------------------------------------
void MarchingCubesExample::cleanup() {

}
