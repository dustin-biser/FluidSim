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
* Interpolate particle attributes onto grid nodes.
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
* @note grid.gridSpec() == weights.gridSpec() must be true or a FluidSim::Exception()
* error will be thrown.
*
* @note Assumes attributes and positions are the same size.
*/
template <typename T>
void interpParticlesToGrid(
		Grid<T> & grid,
		Grid<T> & weights,
		const std::vector<vec2> & positions,
		const std::vector<T> & attributes,
		std::function<float32(const vec2 &, float32)> kernel,
		float32 h
);


/**
* Interpolate grid attributes onto particles
*
* @param[out] attributes  Grid attributes to be interpolated. Specifically,
* attributes[i] is the value interpolated at positions[i] from grid.
*
* @param[in] positions  Particle positions.
*
* @param[in] grid  Grid that attributes are interpolated from.
*
* @param[in] interp  Interpolation function.
*
*
* @note Assumes attributes and positions are the same size.
*/
template <typename T>
void interpGridToParticles(
		std::vector<T> & attributes,
		const std::vector<vec2> & positions,
		const Grid<T> & grid,
		std::function < T (const Grid<T> &, const vec2 &) > interp
);


} // end namespace FluidSim


#include "ParticleGridInterp.inl"
