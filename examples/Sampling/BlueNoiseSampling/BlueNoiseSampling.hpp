// BlueNoiseSampling.hpp

#pragma once

#include "GlfwOpenGlWindow.hpp"
#include "Renderer.hpp"

#include "FluidSim/NumericTypes.hpp"
#include "Timer.hpp"

#include <vector>


const int kScreenWidth = 768;
const int kScreenHeight = 768;


class BlueNoiseSampling : public GlfwOpenGlWindow {

public:
	BlueNoiseSampling();

	~BlueNoiseSampling();

	static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
	std::vector<vec2> samples;

	Timer timer;
	Renderer * renderer;

	virtual void init();
	virtual void logic();
	virtual void draw();
	virtual void keyInput(int key, int action, int mods);
	virtual void cleanup();
};
