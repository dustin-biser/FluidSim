// MarkerFluid.hpp

#pragma once

#include "GlfwOpenGlWindow.hpp"
#include "Renderer.hpp"

const int kScreenWidth = 768;
const int kScreenHeight = 768;


class MarkerFluid : public GlfwOpenGlWindow {

public:
	MarkerFluid();

	~MarkerFluid();

	static std::shared_ptr<GlfwOpenGlWindow> getInstance();

private:
	Renderer * renderer;

	virtual void init();
	virtual void logic();
	virtual void draw();
	virtual void keyInput(int key, int action, int mods);
	virtual void cleanup();
};
