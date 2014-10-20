#include "Grid.hpp"
#include <utility>

namespace FluidSim {

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::Grid()
    : m_data(nullptr),
      m_height(0),
      m_width(0),
      m_cellLength(0),
      m_origin()
{
    
}
       
//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::Grid(
        uint32 width,
        uint32 height,
        float32 cellLength,
        vec2 origin)

    : m_height(height),
      m_width(width),
      m_cellLength(cellLength),
      m_origin(origin)
{
    m_data = new T[width * height];
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::Grid(const Grid<T> & other)
    : m_width(other.m_width),
      m_height(other.m_height),
      m_cellLength(other.m_cellLength),
      m_origin(other.m_origin)
{
    //-- Perform deep copy:
    uint32 num_elements = m_width * m_height;
    m_data = new T [num_elements];
    std::memcpy(m_data, other.m_data, sizeof(T)*num_elements);
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::Grid(Grid<T> && other)
    : m_data(other.m_data),
      m_width(other.m_width),
      m_height(other.m_height),
      m_cellLength(other.m_cellLength),
      m_origin(other.m_origin)
{
    other.m_data = nullptr;
    other.m_height = 0;
    other.m_width = 0;
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::~Grid() {
    delete [] m_data;
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
vec2 Grid<T>::origin() const {
    return m_origin;
}

//---------------------------------------------------------------------------------------
template <typename T>
float32 Grid<T>::cellLength() const {
    return m_cellLength;
}

//---------------------------------------------------------------------------------------
template <typename T>
vec2 Grid<T>::getPosition(uint32 col, uint32 row) {
    vec2 coords(col,row);
    coords *= m_cellLength;
    return m_origin + coords;
}

//---------------------------------------------------------------------------------------
template <typename T>
T & Grid<T>::operator () (uint32 col, uint32 row) const {
    return m_data[m_width*row + col];
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T> & Grid<T>::operator = (Grid<T> && other) {
    m_data = other.m_data;
    m_height = other.m_height;
    m_width = other.m_width;
    m_cellLength = other.m_cellLength;
    m_origin = other.m_origin;

    other.m_data = nullptr;
    other.m_height = 0;
    other.m_width = 0;

    return *this;
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T> & Grid<T>::operator = (const Grid<T> & other) {
    m_width = other.m_width;
    m_height = other.m_height;
    m_cellLength = other.m_cellLength;
    m_origin = other.m_origin;

    uint32 num_elements = m_width * m_height;
    m_data = new T [num_elements];
    std::memcpy(m_data, other.m_data, sizeof(T)*num_elements);

    return *this;
};

//---------------------------------------------------------------------------------------
template <typename T>
void Grid<T>::setAll(const T & val) {
    for(int i(0); i < m_height*m_width; ++i) {
        m_data[i] = val;
    }
}

//---------------------------------------------------------------------------------------
template <typename T>
const T * Grid<T>::data() const {
    return m_data;
}

} // end namespace FluidSim
