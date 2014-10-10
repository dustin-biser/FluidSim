/**
* @brief SmokeSim.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/NumericTypes.hpp"
#include "Grid.hpp"

#include "Utils/GlfwOpenGlWindow.hpp"

#include <glm/glm.hpp>

using namespace FluidSim;
using namespace glm;

//----------------------------------------------------------------------------------------
// Simulation Parameters
//----------------------------------------------------------------------------------------
const float32 kDt = 1 / 60.0f;

//----------------------------------------------------------------------------------------
// Fluid Parameters
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Grid Parameters
//----------------------------------------------------------------------------------------
const int32 kGridWidth = 400;
const int32 kGridHeight = 400;
const float32 kDx = 0.01f; // Grid cell length in meters.
const float32 inv_kDx = 1.0f / kDx;



class SmokeSim : public GlfwOpenGlWindow {

public:
    ~SmokeSim() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    SmokeSim(); // Singleton. Prevent direct construction.

    Grid<vec2> m_velocity;
    Grid<float32> m_pressure;

    GLuint m_tex2D_ink;

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void keyInput(int key, int action, int mods);
    virtual void cleanup();

    void createTextures();

    typedef float TimeStep;
    template <typename U, typename V>
    void advect(const Grid<U> & velocityField, TimeStep dt, Grid<V> & quantity);
};
