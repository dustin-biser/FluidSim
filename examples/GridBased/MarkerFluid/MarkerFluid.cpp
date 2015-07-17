// MarkerFluid.cpp

#include "MarkerFluid.hpp"
#include "Renderer.hpp"

#include "Utils/Timer.hpp"

#include <FluidSim/BlueNoise.hpp>

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

	vec2 domainMin = vec2(0.05, 0.4);
	vec2 domainMax = vec2(0.35, 0.98);
	uint32 maxSamples = 4000;
	float minSampleDistance = 0.012f;


	timer = new Timer();
	timer->start();
	BlueNoise::distributeSamples(
			domainMin,
			domainMax,
			minSampleDistance,
			maxSamples,
			samples
	);
	timer->stop();

	cout << endl;
	cout << "Time: " << timer->getElapsedTime() << " sec" << endl;
	cout << "NumSamples: " << samples.size() << endl;

	renderer = new Renderer(
			kScreenWidth,
			kScreenHeight,
			kGridWidth,
			kGridHeight,
			maxSamples
	);

	renderer->setSampleRadius(0.0022f);
}

//---------------------------------------------------------------------------------------
void MarkerFluid::setupGridData() {
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
	v.setAll(0.0f);
}

//---------------------------------------------------------------------------------------
void MarkerFluid::logic() {

//	advect();
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
	renderer->renderGrid();
	renderer->renderSamples(samples);
}

//---------------------------------------------------------------------------------------
void MarkerFluid::keyInput(int key, int action, int mods) {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::cleanup() {

}
