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

    template <typename T>
    T bilinear(const Grid<T> & grid, const vec2 & worldPos);


    template <typename T>
    tvec2<T> bilinear(const StaggeredGrid<T> & grid, const vec2 & worldPos);


	inline float32 linear(const vec2 & x, float32 h);
}

#include "Interp.inl"
