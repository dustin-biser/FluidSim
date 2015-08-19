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
* Splats particle attributes onto grid nodes using kernel weighted averaging.
* At each grid node:
* $$
* u_{i,j} = \frac{ \sum_p u_p k(\vec{x}_p - \vec{x}_{i,j}) }
*                { \sum_p k(\vec{x}_p - \vec{x}_{i,j}) }
* $$
* where u_{i,j} is the computed grid node value at position \vec{x}_{i,j} and
* u_p is the attribute value for particle p at position \vec{x}_p. The kernel function
* k is used to compute weights for each particle.
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
void splatParticlesToGrid(
		Grid<T> & grid,
		Grid<T> & weights,
		const std::vector<vec2> & positions,
		const std::vector<T> & attributes,
		std::function<float32(const vec2 &, float32)> kernel,
		float32 h
);


/**
* Interpolate grid attributes for each particle position given.
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
