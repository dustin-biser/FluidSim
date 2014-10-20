/**
* Advect.hpp
*
* @author Dustin Biser
*/

#ifndef FLUID_SIM_ADVECT_HPP
#define FLUID_SIM_ADVECT_HPP

#include "Grid.hpp"
#include "StaggeredGrid.hpp"

namespace FluidSim {

typedef float32 TimeStep;

template<typename U, typename V>
void advect(const StaggeredGrid<U> & velocity,
            Grid<V> & quantity,
            TimeStep dt);

} // end namespace FluidSim

#include "Advect.inl"

#endif // FLUID_SIM_ADVECT_HPP
