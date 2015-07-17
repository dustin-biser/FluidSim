// MarkerFluid.hpp

#pragma once


#include "Utils/GlfwOpenGlWindow.hpp"

#include <FluidSim/Grid.hpp>

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

const int32 kGridWidth = 20;
const int32 kGridHeight = 20;
const float32 kGridCellLength = 1.0f / kGridWidth;


class MarkerFluid : public GlfwOpenGlWindow {

public:
	MarkerFluid();

	~MarkerFluid();

	static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
	FluidSim::Grid<float32> u; // x-component of velocity.
	FluidSim::Grid<float32> v; // y-component of velocity.

	std::vector<vec2> samples;

	Timer * timer;
	Renderer * renderer;

	virtual void init();
	virtual void logic();
	virtual void draw();
	virtual void keyInput(int key, int action, int mods);
	virtual void cleanup();

	void setupGridData();

};
