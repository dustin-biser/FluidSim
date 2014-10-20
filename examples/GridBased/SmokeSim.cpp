#include "SmokeSim.hpp"
#include "SmokeGraphics.hpp"
#include "Utils.hpp"
#include "Interp.hpp"
#include "Advect.hpp"

#include <cmath>

#include <glm/glm.hpp>

#include <iostream>
using namespace std;

//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> smokeDemo = SmokeSim::getInstance();
    smokeDemo->create(kScreenWidth, kScreenHeight, "2D Smoke Simulation");

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
void SmokeSim::initGridData() {
    inkGrid = Grid<float32>(kGridWidth, kGridHeight, kDx, vec2(0,0));
    inkGrid.setAll(0);

    // Put data in inkGrid grid:
    int start_row = 0;
    int end_row = 4;
    int start_col = 0;
    int end_col = 4;
    for(int row(start_row); row < end_row; ++row) {
        for(int col(start_col); col < end_col; ++col) {
            inkGrid(col,row) = 0.8f;
        }
    }

    Grid<float32> u(inkGrid.width()+1,
                    inkGrid.height(),
                    kDx,
                    vec2(0, 0.5*kDx));

    Grid<float32> v(inkGrid.width(),
                    inkGrid.height()+1,
                    kDx,
                    vec2(0.5f*kDx, 0));

    velocityGrid = StaggeredGrid<float32>(std::move(u), std::move(v));
    tmp_velocity = velocityGrid;

    // Set initial velocityGrid components:
    velocityGrid.u.setAll(0);
    velocityGrid.v.setAll(kDt);
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
