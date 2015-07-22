// MarkerFluid.cpp

#include "MarkerFluid.hpp"
#include "Renderer.hpp"

#include "Utils/Timer.hpp"
#include "Utils.hpp"

#include <FluidSim/BlueNoise.hpp>
#include <FluidSim/Advect.hpp>

#include <glm/gtx/norm.hpp>
using glm::uvec2;
using glm::ivec2;

#include <iostream>

using namespace std;
using namespace FluidSim;

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
	tmp_velocity = velocityGrid;
	advect(tmp_velocity.u, velocityGrid, kDt);
	advect(tmp_velocity.v, velocityGrid, kDt);
	velocityGrid = tmp_velocity;
}

//---------------------------------------------------------------------------------------
static inline float32 Wpoly6 (
		float32 r2 // distance squared
) {
	// No normalization constant is used here, since both the numerator and denominator of
	// the interpolant are using the same Wpoly6 kernel, so the constant cancels out.

	static const double h = kGridCellLength; // kernel support radius.
	static const double h2 = h*h;

	double x = max(h2 - r2, 0.0);

	return float32(x * x * x);
}

//---------------------------------------------------------------------------------------
static void interpolateVelocityOntoGridNode (
		const ivec2 & cellIndex,
		const vec2 & particlePos,
		float32 particleVel,
		Grid<float32> & numerator_accum,
		Grid<float32> & denominator_accum
) {
	if ( !numerator_accum.isValidCoord(cellIndex) ) {
		return;
	}

	vec2 gridNodePos = numerator_accum.getPosition(cellIndex);

	// Distance squared between particle and grid node.
	float32 r2 = glm::length2(gridNodePos - particlePos);

	float32 Wpoly6_weight = Wpoly6(r2);

	numerator_accum(cellIndex.x, cellIndex.y) += particleVel * Wpoly6_weight;
	denominator_accum(cellIndex.x, cellIndex.y) += Wpoly6_weight;
}

//---------------------------------------------------------------------------------------
// Divides numerator by denominator and stores result in dest.
static void divide(
	Grid<float32> & dest,
	const Grid<float32> & numerator,
	const Grid<float32> & denominator
) {
	for (int j(0); j < dest.height(); ++j) {
		for (int i(0); i < dest.width(); ++i) {
			dest(i,j) = numerator(i,j) / denominator(i,j);
		}
	}
}

//---------------------------------------------------------------------------------------
void MarkerFluid::transferParticlesVelocitiesToGrid() {
	Grid<float32> & u_numerator_accum = velocityGrid.u;
	Grid<float32> & v_numerator_accum = velocityGrid.v;

	Grid<float32> & u_denominator_accum = tmp_velocity.u;
	Grid<float32> & v_denominator_accum = tmp_velocity.v;

	// Set all grid nodes to zero.
	u_numerator_accum.setAll(0.0f);
	u_denominator_accum.setAll(0.0f);
	v_numerator_accum.setAll(0.0f);
	v_denominator_accum.setAll(0.0f);

	ivec2 u_indexOffset[] = {
		{0,-1}, {0,0}, {0,1}, {1,-1}, {1,0}, {1,1}
	};

	ivec2 v_indexOffset[] = {
		{-1,0}, {-1,1}, {0,0}, {0,1}, {1,0}, {1,1}
	};

	ivec2 cellIndex;
	vec2 gridNodePos;
	vec2 particleVel; // particle velocity
	uint32 particleIndex = 0;
	for(const vec2 & particlePos : particlePositions) {
		//-- Take floor of particlePos to get index of grid cell containing particle.
		cellIndex.x = uint32(particlePos.x);
		cellIndex.y = uint32(particlePos.y);

		//-- Interpolate particle velocity onto neighboring u/v velocity grid nodes:
		for (int i(0); i < 6; ++i) {
			particleVel = particleVelocities[particleIndex];

			interpolateVelocityOntoGridNode(
					cellIndex + u_indexOffset[i],
					particlePos,
					particleVel.x,
					u_numerator_accum,
					u_denominator_accum
			);

			interpolateVelocityOntoGridNode(
					cellIndex + v_indexOffset[i],
					particlePos,
					particleVel.y,
					v_numerator_accum,
					v_denominator_accum
			);
		}
		++particleIndex;
	}


	// Set velocity grids node equal to numerator_accum dividied by denominator_accum.
	divide(velocityGrid.u, u_numerator_accum, u_denominator_accum);
	divide(velocityGrid.v, v_numerator_accum, v_denominator_accum);
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
