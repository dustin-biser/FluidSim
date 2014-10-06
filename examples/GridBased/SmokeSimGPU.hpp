/**
* @brief SmokeSim.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "Utils/GlfwOpenGlWindow.hpp"

//----------------------------------------------------------------------------------------
// Simulation Parameters
//----------------------------------------------------------------------------------------
const int kGridWidth = 400;
const int kGridHeight = 400;

//----------------------------------------------------------------------------------------
// Fluid Parameters
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Grid Parameters
//----------------------------------------------------------------------------------------

class SmokeSimGPU : public GlfwOpenGlWindow {

public:
    ~SmokeSimGPU() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    SmokeSimGPU(); // Singleton. Prevent direct construction.

    GLuint tex2D_velocity;
    GLuint tex2D_pressure;
    GLuint tex2D_ink;

    GLuint fbo; // Framebuffer Object

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void keyInput(int key, int action, int mods);
    virtual void cleanup();

    void createTextures();
    void createFBO();
};
