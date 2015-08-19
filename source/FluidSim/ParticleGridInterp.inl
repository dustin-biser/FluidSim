
#include <vector>
#include <functional>

#include "FluidSim/Grid.hpp"
#include "FluidSim/Exception.hpp"
#include "FluidSim/Utils.hpp"



namespace FluidSim {

//---------------------------------------------------------------------------------------
template<typename T>
static void checkGridSizesEqual(
		const Grid<T> & a,
		const Grid<T> & b
) {
	if (a.gridSpec() != b.gridSpec()) {
		throw FluidSim::Exception("GridSpecs do not match.");
	}

}
//---------------------------------------------------------------------------------------
template<typename T>
void adjustWeights(
		Grid<T> & weights
) {
	// Replace all weights close to zero with 1.0.
	const uint32 grid_height = weights.height();
	const uint32 grid_width = weights.width();
	T value;
	T w;
	for (int i(0); i < grid_width; ++i) {
		for (int j(0); j < grid_height; ++j) {
			w = weights(i,j);
			value = std::abs(w) < EPSILON ? 1.0f : w;

			weights(i,j) = value;
		}
	}
}

//---------------------------------------------------------------------------------------
template<typename T>
void splatParticlesToGrid(
		Grid<T> & grid,
		Grid<T> & weights,
		const std::vector<vec2> & positions,
		const std::vector<T> & attributes,
		std::function<float32(const vec2 &, float32)> kernel,
		float32 h
) {
	checkGridSizesEqual(grid, weights);

	// Set all grid nodes to zero.
	grid.setAll(0.0f);
	weights.setAll(0.0f);

	// Max index offset to grid node that a particle can contribute to.
	uint32 maxNeighborOffset( uint32(ceil(h / grid.cellLength())) );

	uvec2 cellIndex;
	uvec2 neighborIndex;
	vec2 gridNodePos;
	T attribute; // Particle attribute
	float32 w;   // Interpolation weight
	uint32 particleIndex(0);
	for (const vec2 & particlePos : positions) {
		cellIndex = grid.gridCoordOf(particlePos);

		//-- Interpolate particle attribute onto neighboring grid nodes:
		for (uint32 i(0); i <= maxNeighborOffset; ++i) {
			for (uint32 j(0); j <= maxNeighborOffset; ++j) {
				neighborIndex = cellIndex + uvec2(i, j);
				gridNodePos = grid.getPosition(neighborIndex);

				w = kernel(gridNodePos - particlePos, h);
				attribute = attributes[particleIndex];

				grid(neighborIndex) += attribute * w;
				weights(neighborIndex) += w;
			}
		}
		++particleIndex;
	}

	adjustWeights(weights);
	grid /= weights;
}

//---------------------------------------------------------------------------------------
template <typename T>
void interpGridToParticles(
		std::vector<T> & attributes,
		const std::vector<vec2> & positions,
		const Grid<T> & grid,
		std::function < T (const Grid<T> &, const vec2 &) > interp
) {

	uint32 index = 0;
	for(const vec2 & position : positions) {
		attributes[index] = interp(grid, position);
		++index;
	}

}


} // end namespace FluidSim
