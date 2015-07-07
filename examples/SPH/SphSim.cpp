#include "SphSim.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/fast_square_root.hpp>
using namespace glm;

#include <iostream>
using namespace std;

#include <cstdlib>

//----------------------------------------------------------------------------------------
int main() {
    shared_ptr<GlfwOpenGlWindow> sphDemo = SphSim::getInstance();
    sphDemo->create(kScreenWidth, kScreenHeight, "SPH Fluid Simulation");

    return 0;
}

//---------------------------------------------------------------------------------------
SphSim::SphSim() {
}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> SphSim::getInstance() {
    static GlfwOpenGlWindow * instance = new SphSim();
    if (p_instance == nullptr) {
        p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
    }

    return p_instance;
}

//----------------------------------------------------------------------------------------
void SphSim::init() {
    cout << "\nInitializing Simulation." << endl;

    InitializeWalls();
    InitializeParticles();

    sphGraphics.init(particles);
}
//----------------------------------------------------------------------------------------
void SphSim::InitializeWalls() {
    walls.resize(kNumWalls);

    walls[0] = Wall(vec2(1,0), 0);
    walls[1] = Wall(vec2(0,1), 0);
    walls[2] = Wall(vec2(-1,0), kViewWidth);
    walls[3] = Wall(vec2(0,-1), kViewHeight);
}

//----------------------------------------------------------------------------------------
void SphSim::InitializeParticles() {
    particles.resize(kNumParticles, Particle());

    const float32 dx = 2*kParticleRadius;
    const float32 dy = 2*kParticleRadius;
    const float32 particlesPerRow = 30;
    float32 x_start = kViewWidth / 6.0f;
    float32 x = x_start;
    float32 y = kViewHeight / 3.0f;

//    srand(0);
//    const int32 MAX_INIT_VELOCITY = 5;

    Particle * p;
    for(uint32 i=0, j=1; i < kNumParticles; ++i, ++j) {
        p = &particles[i];
        p->mass = kParticleMass;
        p->position = vec2(x, y);
        p->next = nullptr;

        // Sudo-Random initial velocity
//        float32 e = i*0.001f + j*0.002f;
//        float32 vx_rand = rand() % MAX_INIT_VELOCITY + e;
//        float32 vy_rand = rand() % MAX_INIT_VELOCITY - e;
//        p->velocity = vec2(vx_rand, vy_rand);

        x += dx;

        if (j == particlesPerRow) {
            y += dy;
            x = x_start;
            j = 0;
        }
    }
}

//----------------------------------------------------------------------------------------
// Returns grid cell coordinates for the location of the particle.
static void ComputeGridCoordates(const Particle & p, GridCoord & outGridCoord) {
    static const float32 one_over_kCellSize = 1.0f / kCellSize;

    outGridCoord.col = p.position.x * one_over_kCellSize;
    outGridCoord.row = p.position.y * one_over_kCellSize;
}

//----------------------------------------------------------------------------------------
void SphSim::UpdateGrid() {
    grid.clear();

    GridCoord gridCoord;
    for(Particle & p : particles) {
        ComputeGridCoordates(p, gridCoord);
        Particle * p_head = grid.at(gridCoord);

        // Prevent p and p.next from referencing the same object.
        if (p_head != &p) {
            // Attach p to the beginning of the particle list at grid cell (x,y),
            // while retaining other Particles in the list.
            p.next = p_head;
            grid.at(gridCoord) = &p;
        }
    }
}

//----------------------------------------------------------------------------------------
/*
 * Apply gravity force to all particles.
 */
void SphSim::ApplyBodyForces() {
    for(Particle &p : particles) {
        p.velocity.y -= 9.8f * kDt;
    }
}

//----------------------------------------------------------------------------------------
void SphSim::AdvanceParticles() {
    for(Particle &p : particles) {
        // Constrain velocity to prevent unbound values.
        {
            p.velocity.x = (p.velocity.x > 0) ?
		            std::min(p.velocity.x,kMaxVelocityComponent) :
                    std::max(p.velocity.x, -kMaxVelocityComponent);

            p.velocity.y = (p.velocity.y > 0) ?
		            std::min(p.velocity.y, kMaxVelocityComponent) :
					std::max(p.velocity.y, -kMaxVelocityComponent);
        }

        p.position.x += p.velocity.x * kDt;
        p.position.y += p.velocity.y * kDt;
    }
}

//----------------------------------------------------------------------------------------
void SphSim::ResolveWallCollisions() {
    for(Particle &p : particles) {
        for(const Wall &wall : walls) {
            vec2 n = wall.normal;
            // Distance between particle and wall along wall's normal.
            float32 distanceToWall = wall.d + dot(p.position, n);

            if (distanceToWall < kParticleRadius) {
                // Particle is intersecting wall.

                // Move particle out of wall by projecting the particle's position
                // along the wall's surface normal.
                float32 overlap = kParticleRadius - distanceToWall;
                p.position += overlap * n;

                float32 relativeVelocity = dot(n, p.velocity);
                if (relativeVelocity < 0.0f) {
                    // Particle is approaching wall.
                    p.velocity -= (1 + kCoefficientOfRestitution ) *
                                  dot(p.velocity, n) * n;
                }
            }
        }
    }
}

//----------------------------------------------------------------------------------------
/**
* Computes the grid coordinates for Particle p's current grid cell,
* and all adjacent cells, and stores them in the array outNeighborGridCoords.
*
* Valid Grid Coords are placed in the front of outNeighborGridCoords with the Particle p's
* current cell at outNeighborGridCoords[0].  The number of valid GridCoords are placed in
* outNumValidGridCoords.
*/
void SphSim::ComputeNeighborGridCoords(const Particle & p,
                                         GridCoord (& outNeighborGridCoords)[kGridCellNeighbors],
                                         uint32 & outNumValidGridCoords) {
    GridCoord gridCoord;
    ComputeGridCoordates(p, gridCoord);

    uint32 n = 0;

    // Place p's coordinates first in array, so p's cell is check first for neighbors.
    outNeighborGridCoords[n++] = gridCoord;

    for(int32 row = gridCoord.row - 1; row <= gridCoord.row + 1; ++row) {
        for(int32 col = gridCoord.col - 1; col <= gridCoord.col + 1; ++col) {
            if(col == gridCoord.col && row == gridCoord.row)
                continue;

            if ( (col >= 0) && (col < kGridWidth) ) {
                if ( (row >= 0) && (row < kGridHeight) ) {
                    outNeighborGridCoords[n++] = GridCoord(col, row);
                }
            }
        }
    }

    outNumValidGridCoords = n;
}

//----------------------------------------------------------------------------------------
/**
* For each Particle, determines the neighbors within a distance kH and copies their data
* into the \c neighbors array for fast lookup later.
*
* Neighbors are only considered that reside in the Particle's current cell, and all
* adjacent cells.
*/
void SphSim::UpdateNeighbors() {

    // TODO Dustin -  See if it's possible to sample from different grid cells
    // uniformly, rather than getting all particles from a cell before moving to
    // the next cell. This could help improve accuracy since only kGridCellNeighbors
    // are used in density computation.
    // 1. Enqueue each non-null Particle * of grid for each neighbor grid coord.
    // 2. while(queue is not empty)
    //      2a. Dequeue the next Particle *.
    //      2b. Do computation using the Particle.
    //      2c. If Particle->next is non-null, enqueue it.

    GridCoord neighborGridCoords[kGridCellNeighbors];
    uint32 numValidGridCoords;

    for(uint32 p_index = 0; p_index < kNumParticles; ++p_index) {
        Particle & pi = particles[p_index];

        ComputeNeighborGridCoords(pi, neighborGridCoords, numValidGridCoords);

        uint32 n_index = 0; // neighbor index

        // Loop over all adjacent cells of Particle pi, including pi's own cell.
        for(uint32 i = 0; i < numValidGridCoords; ++i) {
            const GridCoord & gridCoord = neighborGridCoords[i];

            // Loop over all particles within each grid cell.
            for(Particle * pj = grid.at(gridCoord); // pj are neighbors to Particle pi.
                pj != nullptr && n_index < kMaxNeighborParticles;
                pj = pj->next) {

                float32 dx = pi.position.x - pj->position.x;
                float32 dy = pi.position.y - pj->position.y;
                float32 r2 = dx*dx + dy*dy;

                if (r2 > kH*kH || r2 < kEpsilon)
                    continue;

                // pj is within a distance kH of pi, so count it as a neighbor and store
                // a copy of it for later retrieval.
                neighbors[p_index].particles[n_index] = *pj;
                neighbors[p_index].r[n_index] = std::sqrt(r2);
                ++n_index;
            }
        }

        neighbors[p_index].numActiveNeighbors = n_index;
    }
}

//----------------------------------------------------------------------------------------
void SphSim::ComputeDensity() {
    for(uint32 p_index = 0; p_index < kNumParticles; ++p_index) {
        Particle & pi = particles[p_index];

        pi.density = kMinDensity;

        Neighbors neighborData = neighbors[p_index];

        for(uint32 n_index = 0; n_index < neighborData.numActiveNeighbors; ++n_index) {
            Particle & pj = neighborData.particles[n_index];
            float32 r = neighborData.r[n_index];

            float32 value = (kH * kH) - (r * r);

            pi.density += pj.mass * kNormPoly6 * (value * value * value);

        }
    }
}

//----------------------------------------------------------------------------------------
void SphSim::ComputePressure() {
    for(uint32 p_index = 0; p_index < kNumParticles; ++p_index) {
        Particle & pi = particles[p_index];
        pi.pressure = kStiffness * (pi.density - kRestDensity);
    }
}

//----------------------------------------------------------------------------------------
void SphSim::ApplyInternalForces() {
    for(uint32 p_index = 0; p_index < kNumParticles; ++p_index) {
        Particle & pi = particles[p_index];

        vec2 force_pressure(0.0f, 0.0f);
        vec2 force_viscosity(0.0f, 0.0f);

        Neighbors neighborData = neighbors[p_index];

        for(uint32 n_index = 0; n_index < neighborData.numActiveNeighbors; ++n_index) {
            const Particle & pj = neighborData.particles[n_index];

            float32 r = neighborData.r[n_index];
            vec2 r_dir = (pi.position - pj.position) / r;
            float32 a = (kH - r);

            force_pressure -= pj.mass * (pi.pressure + pj.pressure) / (2 * pj.density) *
                (a * a) * r_dir;

            force_viscosity += pj.mass * (pj.velocity - pi.velocity) / pj.density * a;
        }

        force_pressure *= kNormGradSpiky;
        force_viscosity *= kNormViscosity * kDynamicViscosity;

        pi.velocity += (force_pressure + force_viscosity) * kDt / pi.density;
    }
}

//----------------------------------------------------------------------------------------
void SphSim::logic() {
    ApplyBodyForces();
    AdvanceParticles();
    ResolveWallCollisions();
    UpdateGrid();

    ComputeDensity();
    UpdateNeighbors();

    ComputePressure();

    ApplyInternalForces();
    AdvanceParticles();
    ResolveWallCollisions();
    UpdateGrid();

    sphGraphics.UpdateGL(particles);
}

//----------------------------------------------------------------------------------------
void SphSim::draw() {
    sphGraphics.draw();
}

//----------------------------------------------------------------------------------------
void SphSim::keyInput(int key, int action, int mods) {

}

//----------------------------------------------------------------------------------------
void SphSim::cleanup() {
    cout << "Simulation Clean Up" << " ... Good Bye." << endl;
}
