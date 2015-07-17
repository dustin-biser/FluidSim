// BlueNoise.hpp

#pragma once

#include "FluidSim/NumericTypes.hpp"
#include "FluidSim/Grid.hpp"

#include <vector>


namespace FluidSim {
namespace BlueNoise {

	void distributeSamples (
			glm::vec2 domainMin,
			glm::vec2 domainMax,
			float minSampleDistance,
			uint32 maxSamples,
			std::vector<glm::vec2> & samples,
			uint32 numSubStepCandidates = 30
	);

} // end namespace BlueNoise
} // end namespace FluidSim
