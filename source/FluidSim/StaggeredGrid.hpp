/**
* StaggeredGrid.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/NumericTypes.hpp"
#include "FluidSim/Grid.hpp"
#include "FluidSim/Interp.hpp"

namespace FluidSim {

template <typename T>
using interpFunc = T (*) (const Grid<T> &, float32, float32);

template <typename T>
class StaggeredGrid {
public:
    StaggeredGrid();

    StaggeredGrid(const Grid<T> & u, const Grid<T> & v, interpFunc<T> interp = &bilinear);

    StaggeredGrid(Grid<T> && u, Grid<T> && v, interpFunc<T> interp = &bilinear);

    StaggeredGrid(const StaggeredGrid<T> & other);

    StaggeredGrid(StaggeredGrid<T> && other); // move constructor

    ~StaggeredGrid();

    StaggeredGrid<T> & operator = (StaggeredGrid<T> && other);

    StaggeredGrid<T> & operator = (const StaggeredGrid<T> & other);

    tvec2<T> operator () (float32 col, float32 row) const;

    tvec2<T> operator () (const vec2 & coords) const;


    Grid<T> u; // half cell horizontal components
    Grid<T> v; // half cell vertical components

private:
    interpFunc<T> m_interp;
};

} // end namespace FluidSim.

#include "StaggeredGrid.inl"

