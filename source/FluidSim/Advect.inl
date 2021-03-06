#include "Advect.hpp"

using glm::dvec2;

namespace FluidSim {

//----------------------------------------------------------------------------------------
/**
* Semi-Lagrangian advection of \c quantity, based on \c velocityField.
*/
template<typename U, typename V>
void advect (
		Grid<V> & quantity,
        const StaggeredGrid<U> & velocity,
        TimeStep dt
) {
    Grid<V> & q = quantity;

    dvec2 worldPos; // World location within grid to be updated.
    dvec2 x_p; // World location of the particle that will be at worldPos in dt time.
    dvec2 x_mid; // Temporary
    dvec2 u; // Velocity at a given world position.
    Grid<V> q_new = quantity; // Deep copy.

    for (uint32 row(0); row < q.height(); ++row) {
        for (uint32 col(0); col < q.width(); ++col) {
            worldPos = q.getPosition(col,row);
            u = bilinear(velocity, worldPos);

            // Backtrace to particle location that will end up at worldPos at the next
            // time step dt.
            // Two stage Runge-Kutta:
            x_mid = worldPos - (0.5 * dt * u);
            u = bilinear(velocity, x_mid);
            x_p = worldPos - (dt * u);

            q_new(col, row) = bilinear(q, x_p);
        }
    }

    q = std::move(q_new);
}

} // end namespace FluidSim
