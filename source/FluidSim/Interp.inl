#include "Interp.hpp"

#include <cmath>

using glm::dvec2;
using glm::clamp;

namespace FluidSim {

//----------------------------------------------------------------------------------------
template <typename T>
T bilinear(const Grid<T> & grid, const vec2 & worldPos) {
    dvec2 gridCoords = dvec2(worldPos - grid.origin()) / double(grid.cellLength());

    // Clamp gridCoords to be within valid grid coordinate range.
	glm::dvec2 p = glm::clamp(glm::dvec2(gridCoords),
			dvec2(0,0), dvec2(grid.width()-1, grid.height()-1));

    double x1 = floor(p.x);
    double x2 = ceil(p.x);

    double y1 = floor(p.y);
    double y2 = ceil(p.y);

    double a = (p.x - x1);
    double b = (p.y - y1);

	uint32 ux1 = uint32(x1);
	uint32 ux2 = uint32(x2);
	uint32 uy1 = uint32(y1);
	uint32 uy2 = uint32(y2);

    T fR1( (1.0f - a) * grid(ux1,uy1) + a * grid(ux2,uy1) );
    T fR2( (1.0f - a) * grid(ux1,uy2) + a * grid(ux2,uy2) );

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
