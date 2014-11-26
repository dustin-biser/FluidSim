/**
* @brief FramebufferTest.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/NumericTypes.hpp"
using namespace FluidSim;

#include "Utils/GlfwOpenGlWindow.hpp"

#include <glm/glm.hpp>
using namespace glm;

const int textureStorageWidth = 512;
const int textureStorageHeight = 512;

class FramebufferTest : public GlfwOpenGlWindow {

public:
    ~FramebufferTest() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    FramebufferTest() = default; // Singleton. Prevent direct construction.

    GLuint framebuffer; // Framebuffer Object
    GLuint tex2DColorBuffer;

    void createTextureStorage();
    void setupFramebufferAttachments();

    void init();
    void draw();
    void logic();
//    virtual void keyInput(int key, int action, int mods);
//    virtual void cleanup();
};
