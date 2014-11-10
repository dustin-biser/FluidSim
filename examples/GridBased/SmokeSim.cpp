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
                      0.2f);

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
        gridSpec.origin = vec2(kDx,kDx) * 0.5f; // Store values at grid centers.

        densityGrid = Grid<float32>(gridSpec);
        densityGrid.setAll(0);
    }

    //-- Temperature Grid
    {
        GridSpec gridSpec;
        gridSpec.width = kGridWidth;
        gridSpec.height = kGridHeight;
        gridSpec.cellLength = kDx;
        gridSpec.origin = vec2(kDx,kDx) * 0.5f; // Store values at grid centers.

        temperatureGrid = Grid<float32>(gridSpec);
        temperatureGrid.setAll(temp_0); // Set to ambient temperature
    }

    //-- Pressure Grid
    // Pad the pressure grid by one cell around border to allow for ghost values for
    // enforcing Neumann boundary condition dp/dn = 0.
    {
        GridSpec gridSpec;
        gridSpec.width = kGridWidth+2;
        gridSpec.height = kGridHeight+2;
        gridSpec.cellLength = kDx;
        gridSpec.origin = vec2(kDx,kDx) * 0.5f; // Store values at grid centers.

        pressureGrid = Grid<float32>(gridSpec);
        pressureGrid.setAll(0);
    }

    //-- Divergence Grid
    {
        GridSpec gridSpec;
        gridSpec.width = kGridWidth;
        gridSpec.height = kGridHeight;
        gridSpec.cellLength = kDx;
        gridSpec.origin = vec2(kDx,kDx) * 0.5f; // Store values at grid centers.

        divergenceGrid = Grid<float32>(gridSpec);
        divergenceGrid.setAll(0);
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
        v.setAll(0);

        velocityGrid = StaggeredGrid<float32>(std::move(u), std::move(v));

        tmp_velocity = velocityGrid;
    }
}

//----------------------------------------------------------------------------------------
void SmokeSim::advectQuantities() {
    //-- Advect the velocity field
    tmp_velocity = velocityGrid;
    advect(tmp_velocity.u, velocityGrid, kDt);
    advect(tmp_velocity.v, velocityGrid, kDt);
    velocityGrid = tmp_velocity;

    //-- Advect other quantities
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
void SmokeSim::computeDivergence() {
    Grid<float32> & u = velocityGrid.u; // u is (kGridWidth+1 x kGridHeight)
    Grid<float32> & v = velocityGrid.v; // v is (kGridWidth x kGridHeight+1)

    // Compute negative divergence for rhs of Ap = b
    for(int32 row(0); row < kGridHeight; ++row) {
        for (int32 col(0); col < kGridWidth; ++col) {

            float32 delta_u( u(col+1, row) - u(col,row) );
            float32 delta_v( v(col, row+1) - v(col, row) );

            divergenceGrid(col,row) =  -inv_kDx * (delta_u + delta_v);
        }
    }


    //-- Account for Solid Boundary Conditions:
    {
        for(int32 row(0); row < kGridHeight; ++row) {
            // Left boundary
            divergenceGrid(0,row) -=  -inv_kDx * (u(0,row) - u_solid);

            // Right Bounrdary
            divergenceGrid(kGridWidth-1,row) +=  -inv_kDx * (u(kGridWidth,row) - u_solid);
        }

        for(int32 col(0); col < kGridHeight; ++col) {
            // Top boundary
            divergenceGrid(col,kGridHeight-1) +=  -inv_kDx * (v(col,kGridHeight) - v_solid);

            // Bottom boundary
            divergenceGrid(col,0) -=  -inv_kDx * (v(col,0) - v_solid);
        }
    }
}
//----------------------------------------------------------------------------------------
void SmokeSim::computePressure() {
    computeDivergence();

    // Want to solve the system Ap = b
    Grid<float32> & p = pressureGrid;
    Grid<float32> & b = divergenceGrid;

    //-- Set pressure to all zeros for initial guess
    p.setAll(0);

    //-- Apply Gauss-Seidel iterations to Poisson-pressure problem:
    float32 scale = -kDensity * kDx / kDt;
    for(int32 iteration(0); iteration < solver_iterations; ++iteration) {

        // Apply Gauss-Seidel update only to interior cells of pressure grid.
        for(int32 row(1); row < kGridHeight+1; ++row) {
            for(int32 col(1); col < kGridWidth+1; ++col) {

                if ( row == 1 && col == 1 )
                    continue;

                p(col,row) = -scale*b(col-1,row-1) +
                              p(col+1,row) +
                              p(col-1,row) +
                              p(col,row+1) +
                              p(col,row-1);

                p(col,row) *= 0.25;
            }
        }

        //-- Apply Neumann Boundary condition for pressure dp/dn = 0:
        {
            int32 max_width = p.width() - 1;
            int32 max_height = p.height() - 1;

            for (int32 row(0); row < max_height+1; ++row) {
                // Left solid boundary
                // Set each equal to right cell neighbor.
                p(0, row) = p(1, row);

                // Right solid boundary
                // Set each equal to left cell neighbor.
                p(max_width, row) = p(max_width - 1, row);
            }

            for (int32 col(0); col < max_width+1; ++col) {
                // Bottom solid boundary
                // Set each equal to top cell neighbor.
                p(col, 0) = p(col, 1);

                // Top solid boundary
                // Set each equal to bottom cell neighbor.
                p(col, max_height) = p(col, max_height - 1);
            }
        }

        // Dirichlet boundary condition, set one pressure cell to a constant.
        p(1,1) = 0;

    }

}

//----------------------------------------------------------------------------------------
void SmokeSim::subtractPressureGradient() {
    // Here we update the velocity field by subtracting off the pressure gradient
    // making the field "divergence free"/incompressible.

    Grid<float32> & u = velocityGrid.u;
    Grid<float32> & v = velocityGrid.v;
    Grid<float32> & p = pressureGrid;

    float32 scale = -kDt / (kDensity*kDx);
    float32 value;

    //-- Update horizontal velocity:
    for(int32 row(0); row < u.height(); ++row) {
        for(int32 col(0); col < u.width(); ++col) {
            value = scale * p(col+1,row+1);
            u(col,row) -= value;
            u(col+1,row) += value;
        }
    }

    //-- Update vertical velocity:
    for(int32 row(0); row < v.height(); ++row) {
        for(int32 col(0); col < v.width(); ++col) {
            value = scale * p(col+1,row+1);
            v(col,row) -= value;
            v(col,row+1) += value;
        }
    }

    //-- Set boundary velocity to match solid velocity
    {
        // Left and Right horizontal boundary velocities:
        for (int32 row(0); row < u.height(); ++row) {
            u(0,row) = u_solid;
            u(u.width()-1,row) = u_solid;
        }

        // Top and Bottom vertical boundary velocities:
        for (int32 col(0); col < v.width(); ++col) {
            v(col,0) = v_solid;
            v(col,v.height()-1) = v_solid;
        }
    }
}


//----------------------------------------------------------------------------------------
void SmokeSim::logic() {

    //-- Inject density and temperature:
    static uint counter = 0;
    if (counter < 60) {
        // Smoke source:
        fillGrid(densityGrid, 20, 40, 10, 2, 1.0f);
        fillGrid(temperatureGrid, 20, 40, 10, 2, temp_0 + 300);
        counter++;
    }

    advectQuantities();
    addForces();
    computePressure();
    subtractPressureGradient();

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
