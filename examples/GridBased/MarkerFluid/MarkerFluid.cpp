// MarkerFluid.cpp

#include "MarkerFluid.hpp"
#include "Renderer.hpp"

#include "Utils/Timer.hpp"
#include "Utils.hpp"

#include <FluidSim/BlueNoise.hpp>
#include <FluidSim/Advect.hpp>
#include <FluidSim/ParticleGridInterp.hpp>
using namespace FluidSim;

#include <glm/gtx/norm.hpp>

#include <iostream>
using namespace std;

//---------------------------------------------------------------------------------------
int main () {
    shared_ptr<GlfwOpenGlWindow> demo = MarkerFluid::getInstance();
    demo->create(kScreenWidth, kScreenWidth, "Marker Particle Fluid Simulation");
    
    return 0;
}

//---------------------------------------------------------------------------------------
MarkerFluid::MarkerFluid() {

}

//---------------------------------------------------------------------------------------
MarkerFluid::~MarkerFluid() {
	delete timer;
	delete renderer;
}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> MarkerFluid::getInstance() {
	static GlfwOpenGlWindow * instance = new MarkerFluid();
	if (p_instance == nullptr) {
		p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
	}

	return p_instance;
}

//---------------------------------------------------------------------------------------
void MarkerFluid::init() {
	setupGridData();

	uint32 maxParticles = 4000;
	distributeFluidParticles(maxParticles);
	setInitialParticleVelocities();


	renderer = new Renderer(
			kScreenWidth,
			kScreenHeight,
			kGridWidth,
			kGridHeight,
			maxParticles
	);
	renderer->setSampleRadius(0.0022f);
}


//---------------------------------------------------------------------------------------
void MarkerFluid::distributeFluidParticles (
		uint32 maxParticles
) {
	vec2 domainMin = vec2(0.05, 0.4);
	vec2 domainMax = vec2(0.35, 0.98);
	float minSampleDistance = 0.012f;


	timer = new Timer();
	timer->start();
	BlueNoise::distributeSamples(
			domainMin,
			domainMax,
			minSampleDistance,
			maxParticles,
			particlePositions
	);
	timer->stop();

	if(particleVelocities.size() != particlePositions.size()) {
		particleVelocities.resize(particlePositions.size());
	}

	cout << endl;
	cout << "Time: " << timer->getElapsedTime() << " sec" << endl;
	cout << "NumParticles: " << particlePositions.size() << endl;
}

//---------------------------------------------------------------------------------------
void MarkerFluid::setInitialParticleVelocities() {
	for(vec2 & velocity : particleVelocities) {
		velocity = vec2(0.0f);
	}
}

//---------------------------------------------------------------------------------------
void MarkerFluid::setupGridData() {

	FluidSim::Grid<float32> u; // x-component of velocity.
	FluidSim::Grid<float32> v; // y-component of velocity.

	GridSpec u_gridSpec;
	u_gridSpec.width = kGridWidth + 1;
	u_gridSpec.height = kGridHeight;
	u_gridSpec.cellLength = kGridCellLength;
	u_gridSpec.origin = vec2(0, 0.5*kGridCellLength);

	u = Grid<float32>(u_gridSpec);
	u.setAll(0.0f);


	GridSpec v_gridSpec;
	v_gridSpec.width = kGridWidth;
	v_gridSpec.height = kGridHeight + 1;
	v_gridSpec.cellLength = kGridCellLength;
	v_gridSpec.origin = vec2(0.5f*kGridCellLength, 0);

	v = Grid<float32>(v_gridSpec);

	// Constant velocity downwards:
	v.setAll(-0.05f);

	velocityGrid = StaggeredGrid<float32>(std::move(u), std::move(v));
}

//---------------------------------------------------------------------------------------
void MarkerFluid::advectVelocity() {
	//-- Advect velocity field
	tmp_grid = velocityGrid;
	advect(tmp_grid.u, velocityGrid, kDt);
	advect(tmp_grid.v, velocityGrid, kDt);
	velocityGrid = tmp_grid;
}

//---------------------------------------------------------------------------------------
// Copies velocity components from particleVelocities into partcle velocity caches.
void MarkerFluid::updateParticleVelocityCache() {
	particle_u_velocity_cache.reserve(particleVelocities.size());
	particle_v_velocity_cache.reserve(particleVelocities.size());

	uint32 index = 0;
	for (const vec2 & velocity : particleVelocities) {
		particle_u_velocity_cache[index] = velocity.x;
		particle_v_velocity_cache[index] = velocity.y;
		++index;
	}

}

//---------------------------------------------------------------------------------------
void MarkerFluid::transferParticlesVelocitiesToGrid() {
	updateParticleVelocityCache();

	interpParticleToGrid (velocityGrid.u, tmp_grid.u, particlePositions,
			particle_u_velocity_cache, linear, kGridCellLength );

	interpParticleToGrid (velocityGrid.v, tmp_grid.v, particlePositions,
			particle_v_velocity_cache, linear, kGridCellLength );
}

//---------------------------------------------------------------------------------------
void MarkerFluid::updateParticlePositions() {
	for(vec2 & position : particlePositions) {
		float32 u = bilinear(velocityGrid.u, position);
		float32 v = bilinear(velocityGrid.v, position);

		// Euler update to particle position.
		position += vec2(u,v)*kDt;
	}
}

//---------------------------------------------------------------------------------------
void MarkerFluid::addForces() {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::computeRHS() {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::computePressure() {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::subtractPressureGradient() {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::clampMaxVelocity() {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::computeMaxVelocity() {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::logic() {

	advectVelocity();

	transferParticlesVelocitiesToGrid();

//	updateParticlePositions();

//	addForces();
//
//	computeRHS();
//	computePressure();
//	subtractPressureGradient();
//
//	clampMaxVelocity();
//	computeMaxVelocity();
}

//---------------------------------------------------------------------------------------
void MarkerFluid::draw() {
//	renderer->renderGrid();
	renderer->renderSamples(particlePositions);
}

//---------------------------------------------------------------------------------------
void MarkerFluid::keyInput(int key, int action, int mods) {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::cleanup() {

}
