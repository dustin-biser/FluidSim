/**
* StaggeredGrid.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/NumericTypes.hpp"
#include "FluidSim/Grid.hpp"

namespace FluidSim {

// Forward Declaration
namespace GridInterp {
	template<typename T>
	T linear (
			const Grid<T> & grid,
			const vec2 & position
	);
}


template <typename T>
using interpFunc = T (*) (const Grid<T> &, const vec2 &);



template <typename T>
class StaggeredGrid {
public:
    StaggeredGrid();

    StaggeredGrid(
        const Grid<T> & u,
        const Grid<T> & v,
        interpFunc<T> interp = &GridInterp::linear);

    StaggeredGrid(
        Grid<T> && u,
        Grid<T> && v,
        interpFunc<T> interp = &GridInterp::linear);

    StaggeredGrid(const StaggeredGrid<T> & other);

    StaggeredGrid(StaggeredGrid<T> && other); // move constructor

    ~StaggeredGrid();

    StaggeredGrid<T> & operator = (StaggeredGrid<T> && other);

    StaggeredGrid<T> & operator = (const StaggeredGrid<T> & other);


    Grid<T> u; // Horizontal component grid.
    Grid<T> v; // Vertical component grid.

private:
    interpFunc<T> m_interp;
};

} // end namespace FluidSim.

#include "StaggeredGrid.inl"

