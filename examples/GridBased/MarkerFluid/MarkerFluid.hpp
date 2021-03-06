// MarkerFluid.hpp

#pragma once


#include "Utils/GlfwOpenGlWindow.hpp"

#include <FluidSim/StaggeredGrid.hpp>

#include <vector>

//---------------------------------------------------------------------------------------
// Forward Declarations
//---------------------------------------------------------------------------------------
class Renderer;
class Timer;

//---------------------------------------------------------------------------------------
// Simulation Constants
//---------------------------------------------------------------------------------------
const int kScreenWidth = 768;
const int kScreenHeight = 768;

const int32 kGridWidth = 40;
const int32 kGridHeight = 40;
const float32 kGridCellLength = 1.0f / kGridWidth;

const float32 kDt = 0.02f;

//---------------------------------------------------------------------------------------

class MarkerFluid : public GlfwOpenGlWindow {
public:
	MarkerFluid();

	~MarkerFluid();

	static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
	FluidSim::StaggeredGrid<float32> velocityGrid;
	FluidSim::StaggeredGrid<float32> tmp_grid;

	std::vector<vec2> particlePositions;
	std::vector<vec2> particleVelocities;
	std::vector<float32> particle_u_velocity_cache;
	std::vector<float32> particle_v_velocity_cache;


	Timer * timer;
	Renderer * renderer;

	virtual void init();
	virtual void logic();
	virtual void draw();
	virtual void keyInput(int key, int action, int mods);
	virtual void cleanup();

	void setupGridData();
	void distributeFluidParticles(uint32 maxParticles);
	void setInitialParticleVelocities();

	void advectVelocity();
	void transferParticlesVelocitiesToGrid();
	void updateParticleVelocityCache();


	// TODO Dustin - Implement these methods:
	void updateParticlePositions();
	void addForces();

	void computeRHS();
	void computePressure();
	void projectVelocity();

	void clampMaxVelocity();
	void computeMaxVelocity();

};
