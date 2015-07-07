#include "GlfwOpenGlWindow.hpp"
#include "MarkerFluid.hpp"

using std::shared_ptr;

//---------------------------------------------------------------------------------------
MarkerFluid::MarkerFluid() {

}

//---------------------------------------------------------------------------------------
MarkerFluid::~MarkerFluid() {

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

}

//---------------------------------------------------------------------------------------
void MarkerFluid::keyInput(int key, int action, int mods) {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::cleanup() {

}
