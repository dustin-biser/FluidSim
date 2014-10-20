#include "FluidSim/StaggeredGrid.hpp"
#include "FluidSim/Interp.hpp"
#include <utility>

namespace FluidSim {

//----------------------------------------------------------------------------------------
template <typename T>
StaggeredGrid<T>::StaggeredGrid()
    : u(),
      v(),
      m_interp(&bilinear)
{

}

//----------------------------------------------------------------------------------------
template <typename T>
StaggeredGrid<T>::StaggeredGrid (
        const Grid<T> & u,
        const Grid<T> & v,
        interpFunc<T> interp)
    : u(u),
      v(v),
      m_interp(interp)
{

}

//----------------------------------------------------------------------------------------
template <typename T>
StaggeredGrid<T>::StaggeredGrid(
        Grid<T> && u,
        Grid<T> && v,
        interpFunc<T> interp)
    : u(std::move(u)),
      v(std::move(v)),
      m_interp(interp)
{

}

//----------------------------------------------------------------------------------------
template <typename T>
StaggeredGrid<T>::StaggeredGrid(const StaggeredGrid<T> & other)
    : u(other.u),
      v(other.v),
      m_interp(other.m_interp)
{

}

//----------------------------------------------------------------------------------------
template <typename T>
StaggeredGrid<T>::StaggeredGrid(StaggeredGrid<T> && other)
    : u(std::move(other.u)),
      v(std::move(other.v))
{

}

//----------------------------------------------------------------------------------------
template <typename T>
StaggeredGrid<T>::~StaggeredGrid() {

}

//----------------------------------------------------------------------------------------
template <typename T>
StaggeredGrid<T> & StaggeredGrid<T>::operator = (StaggeredGrid<T> && other) {
    if (&other != this) {
        u = std::move(other.u);
        v = std::move(other.v);
    }

    return *this;
}

//----------------------------------------------------------------------------------------
template <typename T>
tvec2<T> StaggeredGrid<T>::operator () (float32 col, float32 row) const {
    T u_val = m_interp(u, col+0.5f, row);
    T v_val = m_interp(v, col, row+0.5f);

    return tvec2<T>(u_val,v_val);
}

//----------------------------------------------------------------------------------------
template <typename T>
tvec2<T> StaggeredGrid<T>::operator () (const vec2 & coords) const {
    return (*this)(coords.x, coords.y);
}

//----------------------------------------------------------------------------------------
template <typename T>
StaggeredGrid<T> & StaggeredGrid<T>::operator = (const StaggeredGrid<T> & other) {
    u = other.u;
    v = other.v;

    return *this;
}

} // end namespace FluidSim




















































