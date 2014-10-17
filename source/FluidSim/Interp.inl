#include "Interp.hpp"

#include <cmath>

namespace FluidSim {

//----------------------------------------------------------------------------------------
/**
* Applies bilinear interopolation to compute the value of \c grid at location \c coords.
*
* @note
* If \c coords is outside the valid coordinate range of \c grid, then \c coords is clamped
* to within valid \c grid coordinate range before interpolation.
*/
template <typename T>
T bilinear(const Grid<T> & grid, const vec2 & coords) {
    // Clamp coords so they are within valid grid coordinate range.
    vec2 p = glm::clamp(coords, vec2(0,0), vec2(grid.width()-1, grid.height()-1));

    float32 x1 = floor(p.x);
    float32 x2 = ceil(p.x);

    float32 y1 = floor(p.y);
    float32 y2 = ceil(p.y);

    float32 a = (p.x - x1);
    float32 b = (p.y - y1);

    T fR1( (1 - a)*grid(x1,y1) + a*grid(x2,y1) );
    T fR2( (1 - a)*grid(x1,y2) + a*grid(x2,y2) );

    return (1 - b)*fR1 + b*fR2;
}

//----------------------------------------------------------------------------------------
/**
* Applies bilinear interopolation to compute the value of \c grid at location \c (col,
* row).
*
* @note
* If \c (col, row) is outside the valid coordinate range of \c grid, then \c (col, row)
* is clamped to within valid \c grid coordinate range before interpolation.
*/
template <typename T>
T bilinear(const Grid<T> & grid, float32 col, float32 row) {
    return bilinear(grid, vec2(col,row));
}

} // end namespace FluidSim
