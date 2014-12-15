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

    max_vel = vec2(0,0);

    initGridData();

    smokeGraphics.init(densityGrid);
    smokeGraphics.uploadSolidCellData(cellGrid);
}

//----------------------------------------------------------------------------------------
static void fillGrid(Grid<float32> & grid,
                     int32 start_col,
                     int32 col_span,
                     int32 start_row,
                     int32 row_span,
                     float32 value)
{
    for(int32 row(start_row); row < start_row + row_span; ++row) {
        for(int32 col(start_col); col < start_col + col_span; ++col) {
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
        temperatureGrid.setAll(kTemp_0); // Set to ambient temperature
    }

    //-- Cell Grid
    {
        GridSpec gridSpec;
        gridSpec.width = kGridWidth;
        gridSpec.height = kGridHeight;
        gridSpec.cellLength = kDx;
        gridSpec.origin = vec2(kDx,kDx) * 0.5f; // Store values at grid centers.

        cellGrid = Grid<CellType>(gridSpec);
        cellGrid.setAll(CellType::Fluid);

        //-- Set Location of Solid Cells:
        {
            // Boundary Cells are Solid
            for (int32 row(0); row < cellGrid.height(); ++row) {
                cellGrid(0, row) = CellType::Solid;
                cellGrid(cellGrid.width() - 1, row) = CellType::Solid;

            }
            for (int32 col(0); col < cellGrid.width(); ++col) {
                cellGrid(col, 0) = CellType::Solid;
                cellGrid(col, cellGrid.height() - 1) = CellType::Solid;

            }

            // Create a Solid Box near center of grid
            int32 mid_col = kGridWidth / 2;
            int32 mid_row = kGridHeight / 2;
            for (int32 j(-2); j < 2; ++j) {
                for (int32 i(-10); i < 10; ++i) {
                    cellGrid(mid_col + i, mid_row + j) = CellType::Solid;

                }
            }
        }

    }

    //-- Pressure Grid
    {
        GridSpec gridSpec;
        gridSpec.width = kGridWidth;
        gridSpec.height = kGridHeight;
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

        rhsGrid = Grid<float32>(gridSpec);
        rhsGrid.setAll(0);
    }

    //-- Velocity Grid
    {
        GridSpec u_gridSpec;
        u_gridSpec.width = kGridWidth + 1;
        u_gridSpec.height = kGridHeight;
        u_gridSpec.cellLength = kDx;
        u_gridSpec.origin = vec2(0, 0.5*kDx);

        Grid<float32> u(u_gridSpec);
        u.setAll(0);


        GridSpec v_gridSpec;
        v_gridSpec.width = kGridWidth;
        v_gridSpec.height = kGridHeight + 1;
        v_gridSpec.cellLength = kDx;
        v_gridSpec.origin = vec2(0.5f*kDx, 0);

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

           force = -kBuoyant_d * density + kBuoyant_t * (temp - kTemp_0);

           velocityGrid.v(col,row) += kDt * force;
       }
   }

}

//----------------------------------------------------------------------------------------
void SmokeSim::computeRHS() {
    Grid<float32> & u = velocityGrid.u;
    Grid<float32> & v = velocityGrid.v;

    float32 scale = kDensity * kDx / kDt;


    for(int32 row(0); row < cellGrid.height(); ++row) {
        for (int32 col(0); col < cellGrid.width(); ++col) {
            if (cellGrid(col, row) == CellType::Fluid) {
                float32 delta_u( u(col+1, row) - u(col,row) );
                float32 delta_v( v(col, row+1) - v(col, row) );

                rhsGrid(col,row) =  scale * (delta_u + delta_v);
            }
        }
    }


    // Update RHS based on solid boundaries:
    for(int32 row(0); row < cellGrid.height(); ++row) {
        for(int32 col(0); col < cellGrid.width(); ++col) {

            if (cellGrid(col,row) == CellType::Fluid) {

                // Left Neighbor
                if (cellGrid(col-1,row) == CellType::Solid) {
                   rhsGrid(col,row) += scale * (u(col,row) - u_solid);
                }
                // Right Neighbor
                if (cellGrid(col+1,row) == CellType::Solid) {
                    rhsGrid(col,row) -= scale * (u(col+1,row) - u_solid);
                }
                // Bottom Neighbor
                if (cellGrid(col,row-1) == CellType::Solid) {
                    rhsGrid(col,row) += scale * (v(col,row) - v_solid);
                }
                // Top Neighbor
                if (cellGrid(col,row+1) == CellType::Solid) {
                    rhsGrid(col,row) -= scale * (v(col,row+1) - v_solid);
                }

            }

        }
    }
}
//----------------------------------------------------------------------------------------
void SmokeSim::computePressure() {
    // Want to solve the system Ap = b
    Grid<float32> & p = pressureGrid;

    //-- Set pressure to all zeros for initial guess
    p.setAll(0);

    float32 numFluidNeighbors;
    float32 neighborPressureSum;

    //-- Apply Gauss-Seidel iterations to Poisson-pressure problem:
    for(int32 iteration(0); iteration < kJacobiIterations; ++iteration) {

        for (int32 row(0); row < cellGrid.height(); ++row) {
            for (int32 col(0); col < cellGrid.width(); ++col) {

                if(cellGrid(col,row) == CellType::Fluid) {

                    numFluidNeighbors = 0;
                    neighborPressureSum = 0;

                    // Left Neighbor
                    if (cellGrid(col-1,row) == CellType::Fluid) {
                        ++numFluidNeighbors;
                        neighborPressureSum += p(col-1,row);
                    }
                    // Right Neighbor
                    if (cellGrid(col+1,row) == CellType::Fluid) {
                        ++numFluidNeighbors;
                        neighborPressureSum += p(col+1,row);
                    }
                    // Bottom Neighbor
                    if (cellGrid(col, row-1) == CellType::Fluid) {
                        ++numFluidNeighbors;
                        neighborPressureSum += p(col,row-1);
                    }
                    // Top Neighbor
                    if (cellGrid(col, row+1) == CellType::Fluid) {
                        ++numFluidNeighbors;
                        neighborPressureSum += p(col,row+1);
                    }

                    p(col,row) = -1 / numFluidNeighbors *
                            (rhsGrid(col,row) - neighborPressureSum);

                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------
void SmokeSim::subtractPressureGradient() {
    // Here we update the velocity field by subtracting off the pressure gradient
    // making the field "divergence free"/incompressible.

    Grid<float32> & u = velocityGrid.u;
    Grid<float32> & v = velocityGrid.v;
    Grid<float32> & p = pressureGrid;

    float32 scale = kDt / (kDensity*kDx);
    float32 value;

    for(int32 row(0); row < cellGrid.height(); ++row) {
        for(int32 col(0); col < cellGrid.width(); ++col) {

            if (cellGrid(col,row) == CellType::Fluid) {
                value = scale * p(col,row);
                u(col,row) -= value;
                u(col+1,row) += value;

                v(col,row) -= value;
                v(col,row+1) += value;
            }

        }
    }

    for(int32 row(0); row < cellGrid.height(); ++row) {
        for(int32 col(0); col < cellGrid.width(); ++col) {

            if (cellGrid(col,row) == CellType::Solid) {
                u(col,row) = u_solid;
                u(col+1,row) = u_solid;
                v(col,row) = v_solid;
                v(col,row+1) = v_solid;
            }

        }
    }
}

//----------------------------------------------------------------------------------------
void SmokeSim::computeMaxVelocity() {
    Grid<float32> & u = velocityGrid.u;
    Grid<float32> & v = velocityGrid.v;

    for(int32 row(0); row < u.height(); ++row) {
        for(int32 col(0); col < u.width(); ++col) {
            max_vel.x = std::max(u(col,row), max_vel.x);
        }
    }

    for(int32 row(0); row < v.height(); ++row) {
        for(int32 col(0); col < v.width(); ++col) {
            max_vel.y = std::max(v(col,row), max_vel.y);
        }
    }
}

//----------------------------------------------------------------------------------------
void SmokeSim::clampMaxVelocity() {
    Grid<float32> & u = velocityGrid.u;
    Grid<float32> & v = velocityGrid.v;

    float32 max_cfl_velocity = 2.5f * kDx / kDt;

    float32 value;
    for(int32 row(0); row < u.height(); ++row) {
        for(int32 col(0); col < u.width(); ++col) {
            value =  u(col,row);
            u(col,row) = std::min(value, max_cfl_velocity);
        }
    }
    for(int32 row(0); row < v.height(); ++row) {
        for(int32 col(0); col < v.width(); ++col) {
            value =  v(col,row);
            v(col,row) = std::min(value, max_cfl_velocity);
        }
    }

}

//----------------------------------------------------------------------------------------
void SmokeSim::logic() {

    //-- Inject density and temperature:
    static uint counter = 0;
    if (counter < 60) {
        fillGrid(densityGrid, 35, 10, 1, 6, 1.0f);
        fillGrid(temperatureGrid, 35, 10, 1, 6, kTemp_0 + 200);
        counter++;
    }

    advectQuantities();
    addForces();

    computeRHS();
    computePressure();
    subtractPressureGradient();

    clampMaxVelocity();
    computeMaxVelocity();

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
    cout << "max_u: " << max_vel.x << endl;
    cout << "max_v: " << max_vel.y << endl;
    cout << "CFL Condtion, max(velocity) <= " << 5 * kDx / kDt << endl;
    cout << endl;

    cout << "Simulation Clean Up" << endl;

    smokeGraphics.cleanup();

    cout << " ... Good Bye." << endl;
}
