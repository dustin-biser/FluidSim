// BlueNoise.cpp

#include "BlueNoise.hpp"
#include "FluidSim/Utils.hpp"

using namespace FluidSim;
using namespace glm;
using namespace std;

//---------------------------------------------------------------------------------------
// Function declarations. Limit visibility to this file only.
namespace FluidSim {
namespace BlueNoise {

	static void generateFirstSample(
			const vec2 &domainMin,
			const vec2 &domainExtents,
			float &seed,
			FluidSim::Grid<int32> &accel_grid,
			std::vector<uint32> &activeList,
			std::vector<vec2> &samples
	);

	static void setupAccelGrid(
			float minDistance,
			const vec2 &domainMin,
			const vec2 &domainExtents,
			FluidSim::Grid<int32> &accel_grid
	);

	static void distributeNextSample(
			uint32 numSubStepCandidates,
			const vec2 &domainMin,
			const vec2 &domainMax,
			float minDistance,
			float &seed,
			Grid<int32> &accel_grid,
			std::vector<uint32> &activeList,
			std::vector<vec2> &samples
	);

} // end namespace BlueNoise
} // end namespace FluidSim


//---------------------------------------------------------------------------------------
void BlueNoise::setupAccelGrid (
		float minDistance,
		const vec2 & domainMin,
		const vec2 & domainExtents,
		FluidSim::Grid<int32> & accel_grid
) {
	// Grid cell size for neighbor acceleration grid.
	const float cellLength = minDistance / std::sqrt(2.0f);

	// Acceleration grid could stretch past domain, but samples will be clamped
	// to be contained within domain.
	uint32 gridWidth = uint32(ceil( (domainExtents.x) / cellLength ));
	uint32 gridHeight = uint32(ceil( (domainExtents.y) / cellLength ));

	GridSpec gridSpec;
	gridSpec.width = gridWidth;
	gridSpec.height = gridHeight;
	gridSpec.cellLength = cellLength;
	gridSpec.origin = domainMin;

	accel_grid = Grid<int32>(gridSpec);
	accel_grid.setAll(-1);
}

//---------------------------------------------------------------------------------------
void BlueNoise::generateFirstSample (
		const vec2 & domainMin,
		const vec2 & domainExtents,
		float & seed,
		Grid<int32> & accel_grid,
		vector<uint32> & activeList,
		vector<vec2> & samples
) {
	//-- Choose a random sample from the domain
	float32 x_val = randhash(++seed);
	float32 y_val = randhash(++seed);
	vec2 sample = domainMin + vec2(x_val, y_val)*domainExtents;

	ivec2 gridCoord = accel_grid.gridCoordOf(sample);

	accel_grid(gridCoord) = int32(samples.size());
	activeList.push_back(uint32(samples.size()));
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
void BlueNoise::distributeNextSample(
		uint32 numSubStepCandidates,
		const vec2 & domainMin,
		const vec2 & domainMax,
		float minDistance,
		float & seed,
		Grid<int32> & accel_grid,
		vector<uint32> & activeList,
		vector<vec2> & samples
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
	for (int k(0); k < numSubStepCandidates; ++k) {
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
		// No candidate samples were accepted. Remove
		// currentIndex from activeList.
		activeList.pop_back();
	}
}

//---------------------------------------------------------------------------------------
void BlueNoise::distributeSamples (
		glm::vec2 domainMin,
		glm::vec2 domainMax,
		float minSampleDistance,
		uint32 maxSamples,
		std::vector<glm::vec2> & samples,
		uint32 numSubStepCandidates
) {
	samples.clear();

	// Acceleration grid for speeding up sample neighbor search.
	// accel_grid(i,j) = -1, if no sample contained in cell.
	// accel_grid(i,j) = index of sample, if cell contains a sample.
	// No more than one sample can be present per grid cell.
	FluidSim::Grid<int32> accel_grid;

	const vec2 domainExtents = domainMax - domainMin;
	setupAccelGrid(minSampleDistance, domainMin, domainExtents, accel_grid);

	std::vector<uint32> activeList;

	samples.reserve(maxSamples);
	activeList.reserve(maxSamples);

	float seed = 0.0f;
	generateFirstSample(
			domainMin,
			domainExtents,
			seed,
			accel_grid,
			activeList,
			samples
	);


	const uint32 maxIterations = 1000000;
	uint32 iteration = 1;
	while (!activeList.empty() && iteration < maxIterations) {
		distributeNextSample (
				numSubStepCandidates,
				domainMin,
				domainMax,
				minSampleDistance,
				seed,
				accel_grid,
				activeList,
				samples
		);

		++iteration;
	}

	samples.shrink_to_fit();
}
