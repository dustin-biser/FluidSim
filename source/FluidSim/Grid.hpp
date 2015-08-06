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

	bool operator == (const GridSpec & other) const;
	bool operator != (const GridSpec & other) const;
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

	GridSpec gridSpec() const;

	/// Returns true if point p intersects Grid.  Returns false otherwise.
	bool contains(const vec2 & p) const;


	/// Returns the position of grid node at grid index (col,row),
	/// taken into account the Grid origin and cell length.
    vec2 getPosition(uint32 col, uint32 row) const;


	/// Returns the position of grid node at \c index,
	/// taken into account the Grid origin and cell length.
	vec2 getPosition(const glm::uvec2 & index) const;


	/// Returns the grid coordinates of grid cell position p intersects.
	glm::ivec2 gridCoordOf(const vec2 & p) const;


	/// Returns true if col in [0, width-1] and row in [0, height-1].  Returns false
	// otherwise.
	bool isValidCoord(int32 col, int32 row) const;

	/// Returns true if gridCoord.x in [0, width-1] and gridCoord.y in [0, height-1].
	/// Returns false otherwise.
	bool isValidCoord(const glm::ivec2 & gridCoord) const;

    T & operator () (uint32 col, uint32 row) const;

	T & operator () (const glm::uvec2 & gridCoord) const;

    Grid<T> & operator = (Grid<T> && other);

    Grid<T> & operator = (const Grid<T> & other);

	Grid<T> & operator /= (const Grid<T> & other);

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


