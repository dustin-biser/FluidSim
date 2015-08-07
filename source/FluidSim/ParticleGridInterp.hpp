// ParticleGridInterp.hpp

#pragma once

#include "FluidSim/NumericTypes.hpp"

#include <vector>
#include <functional>

// Forward Declaration
namespace FluidSim {
	template<typename T> class Grid;
}


namespace FluidSim {

/**
* Interpolates particle attributes onto grid nodes.
*
*
* @param[out] grid  The destination Grid for which particle attributes
* will be interpolated onto.
*
* @param[out] weights  Sum of interpolation weights computed at each Grid node.
*
* @param[in] positions  Particle positions.
*
* @param[in] attributes  Particle attributes to be interpolated onto grid.
*
* @param[in] kernelFunc  Kernel function used for interpolation.
*
* @param[in] h  Support radius for \c kernelFunc.
*
*
* @note grid.gridSpec() == weights.gridSpec() must be true or an FluidSim::Exception()
* error will be thrown.
*/
template <typename T>
void interpParticleToGrid(
		Grid<T> & grid,
		Grid<T> & weights,
		const std::vector<vec2> & positions,
		const std::vector<T> & attributes,
		std::function<float32 (const vec2 &, float32) > kernel,
		float32 h
);


} // end namespace FluidSim


#include "ParticleGridInterp.inl"
