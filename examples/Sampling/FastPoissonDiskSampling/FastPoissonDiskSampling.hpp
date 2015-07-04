// FastPoissonDiskSampling.hpp

#pragma once

#include "GlfwOpenGlWindow.hpp"
#include "Renderer.hpp"

#include "FluidSim/NumericTypes.hpp"
#include "FluidSim/Grid.hpp"
#include "Timer.hpp"

#include <vector>


const int kScreenWidth = 768;
const int kScreenHeight = 768;


class FastPoissonDiskSampling : public GlfwOpenGlWindow {

public:
	FastPoissonDiskSampling();

	~FastPoissonDiskSampling();

	static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
	vec2 domainMin;
	vec2 domainMax;
	vec2 domainExtents;
	float minDistance;
	float seed;
	std::vector<vec2> samples;
	std::vector<int> activeList;
	FluidSim::Grid<int32> accel_grid;

	Timer timer;
	Renderer * renderer;

	virtual void init();
	virtual void logic();
	virtual void draw();
	virtual void keyInput(int key, int action, int mods);
	virtual void cleanup();

	void distributeNextSample(int maxSamplesPerIteration = 30);

	void generateFirstSample();

	void setupAccelGrid();
};
