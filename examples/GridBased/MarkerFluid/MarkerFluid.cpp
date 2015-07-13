#include "GlfwOpenGlWindow.hpp"
#include "MarkerFluid.hpp"

using namespace FluidSim;

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
	setupGridData();
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

}

//---------------------------------------------------------------------------------------
void MarkerFluid::keyInput(int key, int action, int mods) {

}

//---------------------------------------------------------------------------------------
void MarkerFluid::cleanup() {

}
