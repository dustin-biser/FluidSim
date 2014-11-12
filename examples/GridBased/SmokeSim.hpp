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
using namespace FluidSim;

#include "Utils/GlfwOpenGlWindow.hpp"

#include <glm/glm.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Simulation Parameters
//----------------------------------------------------------------------------------------
const float32 kDt = 0.01;
const int32 solver_iterations = 80;

//----------------------------------------------------------------------------------------
// Fluid Parameters
//----------------------------------------------------------------------------------------
const float32 temp_0 = 273.0f; // Ambient Temperature in Kelvin
const float32 kBuoyant_d = 0.2f; // Density coefficient for buoyant force.
const float32 kBuoyant_t = 0.34f; // Temperature coefficient for buoyant force.
const float32 kDensity = 1.0f;

//----------------------------------------------------------------------------------------
// Grid Parameters
//----------------------------------------------------------------------------------------
// Note: Keep KGridWidth a multiple of 4 so texture alignment is optimal.
const int32 kGridWidth = 100;
const int32 kGridHeight = 100;
const float32 kDx = 0.01f; // Grid cell length in meters.
const float32 inv_kDx = 1.0f/kDx;
const float32 u_solid = 0.0f; // horizontal velocity of solid boundaries.
const float32 v_solid = 0.0f; // vertical velocity of solid boundaries.


class SmokeSim : public GlfwOpenGlWindow {

public:
    ~SmokeSim() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    SmokeSim() = default; // Singleton. Prevent direct construction.

    StaggeredGrid<float32> velocityGrid;
    StaggeredGrid<float32> tmp_velocity;
    Grid<float32> densityGrid;
    Grid<float32> temperatureGrid;
    Grid<float32> pressureGrid;
    Grid<float32> rhsGrid; // rhs of Ap = b

    SmokeGraphics smokeGraphics;

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void keyInput(int key, int action, int mods);
    virtual void cleanup();

    void initGridData();
    void advectQuantities();
    void addForces();
    void computeRHS();
    void computePressure();
    void subtractPressureGradient();

};
