/**
* Grid.hpp
*
* @author Dustin Biser
*/

#pragma once

#include "FluidSim/NumericTypes.hpp"

namespace FluidSim {

struct GridSpec {
    uint32 width;
    uint32 height;
    float32 cellLength;
    vec2 origin;
};

template <typename T>
class Grid {
public:
    Grid();

    Grid(uint32 width,
         uint32 height,
         float32 cellLength,
         vec2 origin);

    Grid(const GridSpec & spec);

    Grid(const Grid<T> & other);

    Grid(Grid<T> && other); // move constructor

    ~Grid();

    uint32 width() const;

    uint32 height() const;

    vec2 origin() const;

    float32 cellLength() const;

    vec2 getPosition(uint32 col, uint32 row) const;

    T & operator () (uint32 col, uint32 row) const;

    Grid<T> & operator = (Grid<T> && other);

    Grid<T> & operator = (const Grid<T> & other);

    void setAll(const T & val);

    const T * data() const;

private:
    T * m_data;
    uint32 m_height;
    uint32 m_width;
    float32 m_cellLength;
    vec2 m_origin;

};

} // end namespace FluidSim.

#include "Grid.inl"


