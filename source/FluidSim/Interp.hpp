/**
* Interp.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "NumericTypes.hpp"
#include "FluidSim/Grid.hpp"
#include "FluidSim/StaggeredGrid.hpp"

namespace FluidSim {

    template <typename T>
    T bilinear(const Grid<T> & grid, const vec2 & worldPos);


    template <typename T>
    tvec2<T> bilinear(const StaggeredGrid<T> & grid, const vec2 & worldPos);

}

#include "Interp.inl"
