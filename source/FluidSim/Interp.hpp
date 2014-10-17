/**
* Interp.hpp
*
* @author Dustin Biser
*/

#ifndef FLUIDSIM_INTERP_HPP
#define FLUIDSIM_INTERP_HPP

#include "NumericTypes.hpp"
#include "Grid.hpp"

namespace FluidSim {

    template <typename T>
    T bilinear(const Grid<T> & grid, const vec2 & coords);

    template <typename T>
    T bilinear(const Grid<T> & grid, float32 col, float32 row);

}

#include "Interp.inl"

#endif // FLUIDSIM_INTERP_HPP
