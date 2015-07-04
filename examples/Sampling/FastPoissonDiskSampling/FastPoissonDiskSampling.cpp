#include "FastPoissonDiskSampling.hpp"
#include "FluidSim/Utils.hpp"
#include "FluidSim/Grid.hpp"

using FluidSim::Utils::randhash;
using FluidSim::Utils::set_array;
using FluidSim::Utils::rand_point_in_annulus;
using FluidSim::Utils::length2;
using FluidSim::Utils::PI;
using FluidSim::GridSpec;
using FluidSim::Grid;

using namespace glm;

#include <chrono>
#include <thread>

//////////////////////////////
// TODO Dustin -  Remove this:
#include <iostream>
using namespace std;
//////////////////////////////



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

	domainMin = vec2(0.0, 0.0);
	domainMax = vec2(1.0, 1.0);
	minDistance = 0.015f;
	seed = 0.0f;
	setupAccelGrid();

	uint32 maxSamples = 4000;
	renderer = new Renderer(
			kScreenWidth,
			kScreenHeight,
			accel_grid.width(),
			accel_grid.height(),
			maxSamples
	);

	renderer->setSampleRadius(0.003);

	timer.start();

		samples.reserve(maxSamples);
		activeList.reserve(maxSamples);

		generateFirstSample();
		while (!activeList.empty()) {
			distributeNextSample(30);
		}
	timer.stop();

	cout << endl;
	cout << "Time: " << timer.getElapsedTime() << " sec" << endl;
	cout << "NumSamples: " << samples.size() << endl;
}

//---------------------------------------------------------------------------------------
void FastPoissonDiskSampling::setupAccelGrid() {

	// Grid cell size for neighbor acceleration grid.
	const float cellLength = minDistance / std::sqrt(2.0f);

	// Acceleration grid could stretch past domain, but samples will be clamped
	// to be contained within domain.
	domainExtents = domainMax - domainMin;
	uint32 gridWidth = uint32(ceil( (domainExtents.x) / cellLength ));
	uint32 gridHeight = uint32(ceil( (domainExtents.y) / cellLength ));

	// accel_grid[i][j] = -1, if no sample contained in cell.
	// accel_grid[i][j] = index of sample, if cell contains a sample.
	// No more than one sample present per grid cell.
	GridSpec gridSpec;
	gridSpec.width = gridWidth;
	gridSpec.height = gridHeight;
	gridSpec.cellLength = cellLength;
	gridSpec.origin = domainMin;

	accel_grid = Grid<int32>(gridSpec);
	accel_grid.setAll(-1);
}

//---------------------------------------------------------------------------------------
void FastPoissonDiskSampling::generateFirstSample() {
	//-- Choose a random sample from the domain
	float32 x_val = randhash(++seed);
	float32 y_val = randhash(++seed);
	vec2 sample = domainMin + vec2(x_val, y_val)*domainExtents;

	ivec2 gridCoord = accel_grid.gridCoordOf(sample);

	accel_grid(gridCoord) = int32(samples.size());
	activeList.push_back(int32(samples.size()));
	samples.push_back(sample);
}

//---------------------------------------------------------------------------------------
// Checks if sample is at least minDistance away from other samples.
// Uses accel_grid to speed up neighbor search.
static bool isSampleSparse (
		const vec2 & sample,
		float minDistance,
		const Grid<int32> & accel_grid,
		const std::vector<vec2> & samples
) {
	// grid coordinate location of sample
	glm::ivec2 gridCoord = accel_grid.gridCoordOf(sample);

	// If there is already a sample present within same grid cell as
	// candidateSample, then candidate is not sparse.
	if (accel_grid(gridCoord) != -1) {
		return false;
	}

	int32 index_i, index_j;
	int32 sampleIndex;
	vec2 neighborSample;
	//-- Check distance from sample to neighboring samples in adjacent cells
	for (int row(-1); row < 2; ++row) {
		for (int col(-1); col < 2; ++col) {
			if (row == 0 && col == 0)
				// No need to check the grid cell sample is located in, since each grid
				// cell contains only one sample.
				continue;

			index_i = gridCoord.x + col;
			index_j = gridCoord.y + row;

			if(!accel_grid.isValidCoord(index_i, index_j))
				// Indices outside range of accel_grid.
				continue;

			sampleIndex = accel_grid(index_i, index_j);
			if (sampleIndex == -1) {
				// No sample within neighboring grid cell (index_i, index_j).
				continue;
			} else {
				neighborSample = samples[sampleIndex];
				if (length2(neighborSample - sample) < minDistance * minDistance) {
					return false;
				}
			}
		}
	}

	// No samples found within minDistance range.
	return true;
}

//---------------------------------------------------------------------------------------
static inline bool isSampleInDomain (
		const vec2 & sample,
		const vec2 & min,
		const vec2 & max
) {
	return (sample.x > min.x) &&
		   (sample.y > min.y) &&
		   (sample.x < max.x) &&
		   (sample.y < max.y);
}

//---------------------------------------------------------------------------------------
void FastPoissonDiskSampling::distributeNextSample(
		int maxSamplesPerIteration
) {
	int currentIndex = activeList.back();
	vec2 activeSample = samples[currentIndex];

	const float innerRadius = minDistance;
	const float outerRadius = 2.0f * minDistance;

	// Generate up to k candidate samples within annulus centered at activeSample.
	// If candidate sample is within minDistance of another existing sample,
	// reject it and continue.
	// Else, add candidate sample to samples, and its index to activeList.
	//
	// If after attempting k candidate samples, none are accepted, remove the
	// current index from activeList.
	vec2 candidateSample;
	bool candidatesAccepted = false;
	for (int k(0); k < maxSamplesPerIteration; ++k) {
		candidateSample = rand_point_in_annulus(activeSample, innerRadius,
				outerRadius, ++seed);

		if ( isSampleInDomain(candidateSample, domainMin, domainMax) ) {
			if ( isSampleSparse(candidateSample, minDistance, accel_grid, samples) ) {
				int32 sampleIndex = int32(samples.size());
				activeList.push_back(sampleIndex);

				ivec2 gridCoord = accel_grid.gridCoordOf(candidateSample);
				accel_grid(gridCoord) = sampleIndex;

				samples.push_back(candidateSample);

				candidatesAccepted = true;
				break;
			}

		} else {
			continue;
		}
	}

	if (!candidatesAccepted) {
		// No candidate samples were accepted, so remove
		// currentIndex from activeList.
		activeList.pop_back();
	}
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

