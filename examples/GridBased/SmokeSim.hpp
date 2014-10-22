/**
* @brief SmokeSim.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "SmokeGraphics.hpp"

#include "FluidSim/NumericTypes.hpp"
#include "FluidSim/Grid.hpp"
#include "FluidSim/StaggeredGrid.hpp"

#include "Utils/GlfwOpenGlWindow.hpp"

#include <glm/glm.hpp>

using namespace FluidSim;
using namespace glm;

//----------------------------------------------------------------------------------------
// Simulation Parameters
//----------------------------------------------------------------------------------------
const float32 kDt = 0.1; //1/60.0f;

//----------------------------------------------------------------------------------------
// Fluid Parameters
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// Grid Parameters
//----------------------------------------------------------------------------------------
// Note: Keep KGridWidth a multiple of 4 so texture alignment is optimal.
 const int32 kGridWidth = 80;
const int32 kGridHeight = 80;
const float32 kDx = 0.01f; // Grid cell length in meters.
const float32 inv_kDx = 1.0f/kDx;



class SmokeSim : public GlfwOpenGlWindow {

public:
    ~SmokeSim() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    SmokeSim(); // Singleton. Prevent direct construction.

    StaggeredGrid<float32> velocityGrid;
    StaggeredGrid<float32> tmp_velocity;
    Grid<float32> pressureGrid;
    Grid<float32> inkGrid;

    SmokeGraphics smokeGraphics;

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void keyInput(int key, int action, int mods);
    virtual void cleanup();

    void initGridData();

};
