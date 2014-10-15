#include "Grid.hpp"
#include <utility>

namespace FluidSim {

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::Grid(uint32 width, uint32 height)
    : m_height(height),
      m_width(width)
{
    data = new T[width * height];
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::Grid(const Grid<T> & other)
    : m_width(other.m_width),
      m_height(other.m_height)
{
    //-- Perform deep copy:
    uint32 num_elements = m_width * m_height;
    data = new T [num_elements];
    std::memcpy(data, other.data, sizeof(T)*num_elements);
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::Grid(Grid<T> && other)
    : data(other.data),
      m_height(other.m_height),
      m_width(other.m_width)
{
    other.data = nullptr;
    other.m_height = 0;
    other.m_width = 0;
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::~Grid() {
    delete [] data;
}

//---------------------------------------------------------------------------------------
template <typename T>
uint32 Grid<T>::width() const {
    return m_width;
}

//---------------------------------------------------------------------------------------
template <typename T>
uint32 Grid<T>::height() const {
    return m_height;
}

//---------------------------------------------------------------------------------------
template <typename T>
T & Grid<T>::operator () (uint32 col, uint32 row) const {
    return data[m_width*row + col];
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T> & Grid<T>::operator = (Grid<T> x) {
    // Parameter x is copy constructed. If x is an rvalue, then the
    // move constructor is called.

    std::swap(*this, x);  // Trade resources with x.
    return *this;
}

} // end namespace FluidSim
