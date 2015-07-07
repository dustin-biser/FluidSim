// BlueNoise.hpp
#pragma once

#include "FluidSim/NumericTypes.hpp"
#include "FluidSim/Grid.hpp"

#include <vector>


namespace FluidSim {

class BlueNoise {
public:
	static void distributeSamples (
			glm::vec2 domainMin,
			glm::vec2 domainMax,
			float  minDistance,
			uint32 maxSamples,
			std::vector<glm::vec2> & samples,
			uint32 numSubStepCandidates = 30
	);


private:
	BlueNoise() = delete;

	~BlueNoise() = delete;

	BlueNoise(const BlueNoise & other) = delete;

	BlueNoise & operator = (const BlueNoise & other) = delete;

	static void generateFirstSample (
			const vec2 & domainMin,
			const vec2 & domainExtents,
			float & seed,
			FluidSim::Grid<int32> & accel_grid,
			std::vector<uint32> & activeList,
			std::vector<vec2> & samples
	);

	static void setupAccelGrid (
			float minDistance,
			const vec2 & domainMin,
			const vec2 & domainExtents,
			FluidSim::Grid<int32> & accel_grid
	);

	static void distributeNextSample(
			uint32 numSubStepCandidates,
			const vec2 & domainMin,
			const vec2 & domainMax,
			float minDistance,
			float & seed,
			Grid<int32> & accel_grid,
			std::vector<uint32> & activeList,
			std::vector<vec2> & samples
	);

};

} // end namespace FluidSim
