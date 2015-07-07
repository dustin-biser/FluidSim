/**
* @brief SphSim.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "SphGraphics.hpp"

#include "Utils/GlfwOpenGlWindow.hpp"
#include "FluidSim/NumericTypes.hpp"
#include "FluidSim/Utils.hpp"
using FluidSim::PI;

#include <glm/glm.hpp>
using glm::vec3;

#include <memory>
using std::shared_ptr;

#include <cmath>
// using pow


//----------------------------------------------------------------------------------------
// Simulation Parameters
//----------------------------------------------------------------------------------------
const float32 kScreenWidth = 800.0f;
const float32 kScreenHeight = 800.0f;
const float32 kViewWidth = 10.0f;
const float32 kViewHeight = 10.0f;
const float32 kAspectRatio = kScreenWidth / kScreenHeight;
const float32 kFrameRate = 100.0f;
const float32 kDt = 1.0f / kFrameRate;
const float32 kEpsilon = 0.000001f;

const uint32 kMaxNeighborParticles = 64;
const uint32 kNumWalls = 4;

//----------------------------------------------------------------------------------------
// Fluid Parameters
//----------------------------------------------------------------------------------------
const uint32 kNumParticles = 2500;
const float32 kParticleMass = 0.0036f;
const float32 kParticleRadius = 0.04f;
const float32 kMinDensity = 9.75f; // mass/volume

const float32 kRestDensity = 10.0f;
const float32 kStiffness = 9000; // R*T from ideal gas law, P*V=m*R*T

const float32 kCoefficientOfRestitution = 0.4f;
const float32 kDynamicViscosity = 28000; //9100;
const float32 kMaxVelocityComponent = 10000000.0f;

//----------------------------------------------------------------------------------------
// Smoothing Kernel Parameters
//----------------------------------------------------------------------------------------
const float32 kH = kParticleRadius * 8.0f; // radius of support.

// Normalization constants so that smoothing kernels integrate to 1 for
// 2D circular integration: r in [0, h], theta in [0, 2PI].
const float32 kNormPoly6 = 4 / (PI * pow(kH,8));
const float32 kNormSpiky = 10 / (PI * pow(kH,5));
const float32 kNormViscosity = 10 / (3 * PI * pow(kH,2));

// Normalization constants for gradient of smoothing kernels.
const float32 kNormGradSpiky = -30 / (PI * pow(kH,5));

//----------------------------------------------------------------------------------------
// Grid Parameters
//----------------------------------------------------------------------------------------
const float32 kCellSize = kH;
const uint32 kGridWidth = uint32(kViewWidth / kCellSize) + 1; // In units of kCellSize.
const uint32 kGridHeight = uint32(kViewHeight / kCellSize) + 1; // In units of kCellSize.
const uint32 kGridCellCount = kGridWidth * kGridHeight;
const uint32 kGridCellNeighbors = 9;


struct Particle {
    vec2 position;
    vec2 velocity;
    float32 mass;
    float32 density;
    float32 pressure;

    Particle * next;
};

struct Wall {
    vec2 normal;
    float32 d; // Distance between origin and wall along wall's normal.

    Wall() { }
    Wall(vec2 normal, float32 d) : normal(normal), d(d) { }
};

struct GridCoord {
    GridCoord()
        : col(0), row(0) { }

    GridCoord(uint32 inCol, uint32 inRow)
        : col(inCol), row(inRow) { }

    uint32 col;
    uint32 row;
};

struct Neighbors {
    // The i-th element is the i-th Neighboring Particle.
    Particle particles[kMaxNeighborParticles];

    // The i-th element of r represents the distance between a target Particle and
    // its i-th neighbor.
    float32 r[kMaxNeighborParticles];

    // Number of neighbors affecting a target Particle.
    // These neighbors will be at the beginning of arrays \c particles and \c r.
    uint32 numActiveNeighbors = 0;
};

/*
* CellGrid is a 2D spatial data structure that is composed of axis aligned cells.  Each
* cell is square and has side length equal to kCellSize.  The grid has dimension equal
* to kGridWidth x kGridHeight.  Grid coordinates are oriented so that the bottom left
* of the grid marks the cell (0,0), where as cell (x,y) is located x cells to the right,
* and y cells up from cell (0,0).
*/
struct CellGrid {
    CellGrid() {
        for(Particle * & p : data) {
            p = nullptr;
        }
    }

    Particle * & at(uint32 col, uint32 row) {
        return data[col + row * kGridWidth];
    }

    Particle * & at(const GridCoord & gridCoord) {
        return at(gridCoord.col, gridCoord.row);
    }

    void clear() {
        for(Particle * & p : data) {
            p = nullptr;
        }
    }

    Particle * data[kGridCellCount];
    const uint32 numRows = kGridHeight;
    const uint32 numColumns = kGridWidth;
};

/**
* Smoothed Paticle Hydrodynamics (SPH) Simulation
*
* Coordinate System: origin at bottom left corner of screen with standard axis
*       +y
*       ^
*       |
*        --> +x
*/
class SphSim : public GlfwOpenGlWindow {

public:
    ~SphSim() { }

    static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
    SphSim(); // Singleton. Prevent direct construction.

    virtual void init();
    virtual void logic();
    virtual void draw();
    virtual void keyInput(int key, int action, int mods);
    virtual void cleanup();

    void InitializeWalls();
    void InitializeParticles();

    void ApplyBodyForces();
    void AdvanceParticles();
    void ResolveWallCollisions();
    void UpdateGrid();
    void UpdateNeighbors();
    void ComputeDensity();
    void ComputePressure();
    void ApplyInternalForces();
    void ComputeNeighborGridCoords(const Particle & p,
                                   GridCoord (& outNeighborGridCoords)[kGridCellNeighbors],
                                   uint32 & outNumValidGridCoords);

    SphGraphics sphGraphics;

    vector<Particle> particles;
    vector<Wall> walls;
    CellGrid grid;
    Neighbors neighbors[kNumParticles];

};
