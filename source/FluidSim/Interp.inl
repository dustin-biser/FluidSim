#include "Interp.hpp"

#include <cmath>

namespace FluidSim {

//----------------------------------------------------------------------------------------
template <typename T>
T bilinear(const Grid<T> & grid, const vec2 & worldPos) {
    vec2 gridCoords = (worldPos - grid.origin()) / grid.cellLength();

    // Clamp gridCoords to be within valid grid coordinate range.
    vec2 p = glm::clamp(gridCoords, vec2(0,0), vec2(grid.width()-1, grid.height()-1));

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
template <typename T>
tvec2<T> bilinear(const StaggeredGrid<T> & grid, const vec2 & worldPos) {
    T uValue = bilinear(grid.u, worldPos);
    T vValue = bilinear(grid.v, worldPos);

    return tvec2<T>(uValue,vValue);
}

} // end namespace FluidSim
