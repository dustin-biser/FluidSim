/**
* Interp.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "NumericTypes.hpp"

// Forward Declaration
namespace FluidSim {
	template <typename T> class Grid;
	template <typename T> class StaggeredGrid;
}


namespace FluidSim {

namespace GridInterp {

	/// Returns interpolated grid value at given position.
	template<typename T>
	T linear (
			const Grid<T> & grid,
			const vec2 & position
	);


	/// Returns interpolated grid value at given position.
	template<typename T>
	tvec2<T> bilinear (
			const StaggeredGrid<T> & grid,
			const vec2 & position
	);
}


namespace InterpKernel {

	/// Linear interpolation kernel, with support radius h.
	inline float32 linear(const vec2 &x, float32 h);

}

} // end namespace FluidSim

#include "Interp.inl"
