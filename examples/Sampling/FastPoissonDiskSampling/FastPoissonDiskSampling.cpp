#include "FastPoissonDiskSampling.hpp"
#include "FluidSim/BlueNoise.hpp"
#include "FluidSim/Utils.hpp"
#include "FluidSim/Grid.hpp"

using namespace FluidSim;
using namespace glm;

#include <iostream>
using namespace std;


//----------------------------------------------------------------------------------------
int main() {
	shared_ptr<GlfwOpenGlWindow> demo = FastPoissonDiskSampling::getInstance();
	demo->create(kScreenWidth,
				 kScreenHeight,
				 "Fast Poisson Disk Sampling",
				 1/60.0f);

	return 0;
}

//---------------------------------------------------------------------------------------
shared_ptr<GlfwOpenGlWindow> FastPoissonDiskSampling::getInstance() {
	static GlfwOpenGlWindow * instance = new FastPoissonDiskSampling();
	if (p_instance == nullptr) {
		p_instance = shared_ptr<GlfwOpenGlWindow>(instance);
	}

	return p_instance;
}

//---------------------------------------------------------------------------------------
FastPoissonDiskSampling::FastPoissonDiskSampling() {

}

//---------------------------------------------------------------------------------------
FastPoissonDiskSampling::~FastPoissonDiskSampling() {
	delete renderer;
}

//---------------------------------------------------------------------------------------
void FastPoissonDiskSampling::init() {
	vec2 domainMin = vec2(0.2, 0.2);
	vec2 domainMax = vec2(0.8, 0.8);
	uint32 maxSamples = 4000;
	float minDistance = 0.012f;


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
void FastPoissonDiskSampling::keyInput(int key, int action, int mods) {

}

//---------------------------------------------------------------------------------------
void FastPoissonDiskSampling::cleanup() {

}

//---------------------------------------------------------------------------------------
void FastPoissonDiskSampling::logic() {

}

//---------------------------------------------------------------------------------------
void FastPoissonDiskSampling::draw() {
//	renderer->renderGrid();
	renderer->renderSamples(samples);
}

