#include "Advect.hpp"
#include "Interp.hpp"

namespace FluidSim {

//----------------------------------------------------------------------------------------
/**
* Semi-Lagrangian advection of \c quantity, based on \c velocityField.
*/
template<typename U, typename V>
void advect(const StaggeredGrid<U> & velocity,
            Grid<V> & quantity,
            GridCellLength dx,
            TimeStep dt)
{
    Grid<V> & q = quantity;

    uint32 q_width = q.width();
    uint32 q_height = q.height();

    assert( q_width + 1 == velocity.u.width() &&
            q_height + 1 == velocity.v.height() );


    float32 inv_dx = 1 / dx;

    vec2 x_g; // Grid location to update.
    vec2 x_p; // Grid location of the particle that will be at x_g.
    vec2 x_mid; // Mid point to help aid in better backtracing of particle location.
    Grid<V> q_new = quantity; // Deep copy.

    for (uint32 row(0); row < q_height; ++row) {
        for (uint32 col(0); col < q_width; ++col) {
            x_g = vec2(col, row);

            // Backtrace to particle location that will end up at x_g at the next
            // time step dt.
            // Two stage Runge-Kutta:
            x_mid = x_g - inv_dx * (0.5f * dt * velocity(x_g));
            x_p = x_g - inv_dx * (dt * velocity(x_mid));

            q_new(col, row) = bilinear(q, x_p);
        }
    }

    // Move the advected grid data into q.
    q = std::move(q_new);
}

} // end namespace FluidSim
