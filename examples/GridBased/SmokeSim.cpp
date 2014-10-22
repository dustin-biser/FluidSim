#include "SmokeSim.hpp"
#include "SmokeGraphics.hpp"

#include "FluidSim/Interp.hpp"
#include "FluidSim/Advect.hpp"

#include <cmath>
#include <iostream>
using namespace std;

#include <glm/glm.hpp>


//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> smokeDemo = SmokeSim::getInstance();
    smokeDemo->create(kScreenWidth,
                      kScreenHeight,
                      "2D Smoke Simulation",
                      1/60.f);

    return 0;
}

//---------------------------------------------------------------------------------------
SmokeSim::SmokeSim() {

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
    
    smokeGraphics.init(inkGrid);
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
    inkGrid = Grid<float32>(kGridWidth, kGridHeight, kDx, vec2(0,0));
    inkGrid.setAll(0);

    // Put data into inkGrid grid:
    fillGrid(inkGrid, 20, 40, 10, 2, 1.0);

    Grid<float32> u(inkGrid.width()+1,
                    inkGrid.height(),
                    kDx,
                    vec2(0, 0.5*kDx));

    Grid<float32> v(inkGrid.width(),
                    inkGrid.height()+1,
                    kDx,
                    vec2(0.5f*kDx, 0));

    velocityGrid = StaggeredGrid<float32>(std::move(u), std::move(v));

    // Set initial velocityGrid components:
    velocityGrid.u.setAll(0);
    velocityGrid.v.setAll(kDx);

    tmp_velocity = velocityGrid;
}


//----------------------------------------------------------------------------------------
void SmokeSim::logic() {
    advect(velocityGrid, tmp_velocity.u, kDt);
    advect(velocityGrid, tmp_velocity.v, kDt);
    velocityGrid = tmp_velocity;

    advect(velocityGrid, inkGrid, kDt);


    // Apply the first 3 operators in Equation 12.
//    u = advect(u);
//    u = diffuse(u);
//    u = addForces(u);
//// Now apply the projection operator to the result.
//    p = computepressureGrid(u);
//    u = subtractpressureGridGradient(u, p);

    smokeGraphics.uploadTextureData(inkGrid);
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
