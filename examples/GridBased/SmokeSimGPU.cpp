#include "SmokeSimGPU.hpp"
#include "SmokeGraphicsGPU.hpp"
#include "Utils.hpp"

#include <glm/glm.hpp>

#include <iostream>
using namespace std;

//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> smokeDemo = SmokeSimGPU::getInstance();
    smokeDemo->create(kScreenWidth, kScreenHeight, "2D Smoke Simulation - GPU Based");

    return 0;
}

//---------------------------------------------------------------------------------------
SmokeSimGPU::SmokeSimGPU() {

}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> SmokeSimGPU::getInstance() {
    static GlfwOpenGlWindow * instance = new SmokeSimGPU();
    if (p_instance == nullptr) {
        p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//----------------------------------------------------------------------------------------
void SmokeSimGPU::init() {
    cout << "\nInitializing Simulation." << endl;

    createTextures();
    createFBO();
}

//----------------------------------------------------------------------------------------
void SmokeSimGPU::createTextures() {
    //--Setup velocity texture.
    // The velocity texture will represent a staggered grid of size (kGridWidth+1,kGridHeight+1).
    // Type: RG16
    // Red Channel - u (horizontal component of velocity).
    // Green Channel - v (vertical component of velocity).
    {
        glGenTextures(1, &tex2D_velocity);
        glBindTexture(GL_TEXTURE_2D, tex2D_velocity);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width = kGridWidth + 1;
        int height = kGridHeight + 1;
        int num_components = 2;
        float initialVelocity [height][width][num_components];
        utils::fillArray(initialVelocity, 0.0f, height, width, num_components);

        // Internally store texture as RG16 - two channels of half-floats:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16, width, height, 0, GL_RG, GL_FLOAT, initialVelocity);
    }

    //--Setup pressure texture.
    // The pressure texture will represent a grid of size (kGridWidth, kGridHeight).
    // Type: R16
    // Red Channel - pressure values.
    {
        glGenTextures(1, &tex2D_pressure);
        glBindTexture(GL_TEXTURE_2D, tex2D_pressure);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width = kGridWidth;
        int height = kGridHeight;
        float initialPressure [width][height];
        utils::fillArray(initialPressure, 0.0f, height, width);

        // Internally store texture as RG16 - two channels of half-floats:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_FLOAT, initialPressure);
    }

    //--Setup ink texture.
    // The ink texture will represent a grid of size (kGridWidth, kGridHeight).
    // Type: R16
    // Red Channel - ink density/opacity
    {
        glGenTextures(1, &tex2D_ink);
        glBindTexture(GL_TEXTURE_2D, tex2D_ink);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        int width = kGridWidth;
        int height = kGridHeight;
        float initialInk [width][height];
        utils::fillArray(initialInk, 0.0f, height, width);

        // Internally store texture as RG16 - two channels of half-floats:
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, width, height, 0, GL_RED, GL_FLOAT, initialInk);
    }

    // Unbind the GL_TEXTURE_2D target.
    glBindTexture(GL_TEXTURE_2D, 0);

    CHECK_GL_ERRORS;
}
//----------------------------------------------------------------------------------------
void SmokeSimGPU::createFBO() {
//    glGenFramebuffers(1, &fbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

}

//----------------------------------------------------------------------------------------
void SmokeSimGPU::logic() {
}

//----------------------------------------------------------------------------------------
void SmokeSimGPU::draw() {
}

//----------------------------------------------------------------------------------------
void SmokeSimGPU::keyInput(int key, int action, int mods) {

}

//----------------------------------------------------------------------------------------
void SmokeSimGPU::cleanup() {
    cout << "Simulation Clean Up" << endl;

    //-- Delete Textures:
    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &tex2D_velocity);
    glDeleteTextures(1, &tex2D_pressure);
    glDeleteTextures(1, &tex2D_ink);

    //-- Delete Framebuffer Object:
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo);

    cout << " ... Good Bye." << endl;
}
