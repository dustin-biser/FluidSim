/**
* Advect.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "Grid.hpp"
#include "StaggeredGrid.hpp"

namespace FluidSim {

typedef double TimeStep;

template<typename U, typename V>
void advect(
		Grid<V> & quantity,
        const StaggeredGrid<U> & velocity,
        TimeStep dt
);

} // end namespace FluidSim

#include "Advect.inl"

