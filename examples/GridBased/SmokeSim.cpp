#include "SmokeSim.hpp"
#include "SmokeGraphics.hpp"

#include "FluidSim/Interp.hpp"
#include "FluidSim/Advect.hpp"

#include <cmath>
#include <iostream>
#include <algorithm>
using namespace std;

#include <glm/glm.hpp>


//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> smokeDemo = SmokeSim::getInstance();
    smokeDemo->create(kScreenWidth,
                      kScreenHeight,
                      "2D Smoke Simulation",
                      1/60.0f);

    return 0;
}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> SmokeSim::getInstance() {
    static GlfwOpenGlWindow * instance = new SmokeSim();
    if (p_instance == nullptr) {
        p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//----------------------------------------------------------------------------------------
void SmokeSim::init() {
    cout << "\nInitializing Simulation." << endl;

    initGridData();
    
    smokeGraphics.init(densityGrid);
}

//----------------------------------------------------------------------------------------
static void fillGrid(const Grid<float32> & grid, int start_col, int col_span,
    int start_row, int row_span, float32 value) {

    for(int row(start_row); row < start_row + row_span; ++row) {
        for(int col(start_col); col < start_col + col_span; ++col) {
            grid(col,row) = value;
        }
    }
}

//----------------------------------------------------------------------------------------
void SmokeSim::initGridData() {

    //-- Density Grid
    {
        GridSpec gridSpec;
        gridSpec.width = kGridWidth;
        gridSpec.height = kGridHeight;
        gridSpec.cellLength = kDx;
        gridSpec.origin = vec2(0,0);

        densityGrid = Grid<float32>(gridSpec);
        densityGrid.setAll(0);
    }

    //-- Temperature Grid
    {
        GridSpec gridSpec;
        gridSpec.width = kGridWidth;
        gridSpec.height = kGridHeight;
        gridSpec.cellLength = kDx;
        gridSpec.origin = vec2(0,0);

        temperatureGrid = Grid<float32>(gridSpec);
        temperatureGrid.setAll(temp_0); // Set to ambient temperature
    }

    //-- Velocity Grid
    {

        GridSpec u_gridSpec;
        u_gridSpec.width = densityGrid.width()+1;
        u_gridSpec.height = densityGrid.height();
        u_gridSpec.cellLength = kDx;
        u_gridSpec.origin = vec2(0, 0.5*kDx);

        Grid<float32> u(u_gridSpec);
        u.setAll(0);


        GridSpec v_gridSpec;
        v_gridSpec.width = densityGrid.width();
        v_gridSpec.height = densityGrid.height()+1;
        v_gridSpec.cellLength = kDx;
        v_gridSpec.origin = vec2(0.5f * kDx, 0);

        Grid<float32> v(v_gridSpec);
        v.setAll(0.01*kDx);

        velocityGrid = StaggeredGrid<float32>(std::move(u), std::move(v));

        tmp_velocity = velocityGrid;
    }
}

//----------------------------------------------------------------------------------------
void SmokeSim::advectQuantities() {
    tmp_velocity = velocityGrid;
    advect(tmp_velocity.u, velocityGrid, kDt);
    advect(tmp_velocity.v, velocityGrid, kDt);
    velocityGrid = tmp_velocity;

    advect(densityGrid, velocityGrid, kDt);
    advect(temperatureGrid, velocityGrid, kDt);
}

//----------------------------------------------------------------------------------------
void SmokeSim::addForces() {

   //-- Buoyant Force:
   float32 force;
   float32 density;
   float32 temp;
   vec2 worldPos;
   for(uint32 row(0); row < kGridHeight; ++row) {
       for(uint32 col(0); col < kGridWidth; ++col) {
           worldPos = velocityGrid.v.getPosition(col,row);
           density = bilinear(densityGrid, worldPos);
           temp = bilinear(temperatureGrid, worldPos);

           force = -kBuoyant_d * density + kBuoyant_t * (temp - temp_0);

           velocityGrid.v(col,row) += kDt * force;
       }
   }

}


//----------------------------------------------------------------------------------------
void SmokeSim::logic() {

    static uint counter = 0;
    if (counter < 60) {
        // Smoke source:
        fillGrid(densityGrid, 20, 40, 10, 2, 1.0f);
        fillGrid(temperatureGrid, 20, 40, 10, 2, temp_0 + 300);
        counter++;
    }

    advectQuantities();
    addForces();

    // Apply the first 3 operators in Equation 12.
//    u = advect(u);
//    u = diffuse(u);
//    u = addForces(u);
//// Now apply the projection operator to the result.
//    p = computepressureGrid(u);
//    u = subtractpressureGridGradient(u, p);

    smokeGraphics.uploadTextureData(densityGrid);
}

//----------------------------------------------------------------------------------------
void SmokeSim::draw() {
    smokeGraphics.draw();
}

//----------------------------------------------------------------------------------------
void SmokeSim::keyInput(int key, int action, int mods) {

}

//----------------------------------------------------------------------------------------
void SmokeSim::cleanup() {
    cout << "Simulation Clean Up" << endl;

    cout << " ... Good Bye." << endl;
}
