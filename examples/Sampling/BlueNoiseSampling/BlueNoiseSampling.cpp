// BlueNoiseSampling.cpp

#include "BlueNoiseSampling.hpp"
#include "FluidSim/BlueNoise.hpp"

using namespace FluidSim;
using namespace glm;

#include <iostream>
using namespace std;


//----------------------------------------------------------------------------------------
int main() {
	shared_ptr<GlfwOpenGlWindow> demo = BlueNoiseSampling::getInstance();
	demo->create(kScreenWidth,
				 kScreenHeight,
				 "Blue Noise Sampling 2D",
				 1/60.0f);

	return 0;
}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> BlueNoiseSampling::getInstance() {
	static GlfwOpenGlWindow * instance = new BlueNoiseSampling();
	if (p_instance == nullptr) {
		p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
	}

	return p_instance;
}

//---------------------------------------------------------------------------------------
BlueNoiseSampling::BlueNoiseSampling() {

}

//---------------------------------------------------------------------------------------
BlueNoiseSampling::~BlueNoiseSampling() {
	delete renderer;
}

//---------------------------------------------------------------------------------------
void BlueNoiseSampling::init() {
	vec2 domainMin = vec2(0.2, 0.2);
	vec2 domainMax = vec2(0.8, 0.8);
	uint32 maxSamples = 5000;
	float minDistance = 0.015f;


	timer.start();
		BlueNoise::distributeSamples(
				domainMin,
				domainMax,
				minDistance,
				maxSamples,
				samples
		);
	timer.stop();

	cout << endl;
	cout << "Time: " << timer.getElapsedTime() << " sec" << endl;
	cout << "NumSamples: " << samples.size() << endl;


	//-- Compute width/height for acceleration grid used for distributing samples.
	const float cellLength = minDistance / std::sqrt(2.0f);
	vec2 domainExtents = domainMax - domainMin;
	uint32 gridWidth = uint32(ceil( (domainExtents.x) / cellLength ));
	uint32 gridHeight = uint32(ceil( (domainExtents.y) / cellLength ));

	renderer = new Renderer(
			kScreenWidth,
			kScreenHeight,
			gridWidth,
			gridHeight,
			maxSamples
	);

	renderer->setSampleRadius(0.0022f);
}

//---------------------------------------------------------------------------------------
void BlueNoiseSampling::keyInput(int key, int action, int mods) {

}

//---------------------------------------------------------------------------------------
void BlueNoiseSampling::cleanup() {

}

//---------------------------------------------------------------------------------------
void BlueNoiseSampling::logic() {

}

//---------------------------------------------------------------------------------------
void BlueNoiseSampling::draw() {
//	renderer->renderGrid();
	renderer->renderSamples(samples);
}

