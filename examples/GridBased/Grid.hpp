/**
* Grid.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/NumericTypes.hpp"

namespace FluidSim {

template <typename T>
class Grid {
public:
    Grid(uint32 width, uint32 height);

    Grid(Grid<T> && other); // move constructor

    ~Grid();

    uint32 width() const;

    uint32 height() const;

    T & operator () (uint32 col, uint32 row) const;

    Grid<T> & operator = (Grid<T> x);

private:
    T * data;
    uint32 m_height;
    uint32 m_width;

};

} // end namespace FluidSim.

#include "Grid.inl"


