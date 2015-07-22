#include "Grid.hpp"
#include <utility>

namespace FluidSim {

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T>::Grid()
    : m_data(nullptr),
      m_height(0.0f),
      m_width(0.0f),
      m_cellLength(0.0f),
      m_origin(vec2(0.0f))
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
Grid<T>::Grid(const GridSpec & spec)
    : m_height(spec.height),
      m_width(spec.width),
      m_cellLength(spec.cellLength),
      m_origin(spec.origin)
{
    m_data = new T[m_width * m_height];
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
vec2 Grid<T>::getPosition(uint32 col, uint32 row) const {
    vec2 coords(col,row);
    coords *= m_cellLength;
    return m_origin + coords;
}


//---------------------------------------------------------------------------------------
template <typename T>
vec2 Grid<T>::getPosition(const glm::uvec2 & index) const {
	return getPosition(index.x, index.y);
}

//---------------------------------------------------------------------------------------
template <typename T>
T & Grid<T>::operator () (uint32 col, uint32 row) const {
	assert(isValidCoord(col, row));

    return m_data[m_width*row + col];
}

//---------------------------------------------------------------------------------------
template <typename T>
T & Grid<T>::operator () (const glm::uvec2 & gridCoord) const {
	assert(isValidCoord(gridCoord.x, gridCoord.y));

	return m_data[m_width*gridCoord.y + gridCoord.x];
}

//---------------------------------------------------------------------------------------
template <typename T>
Grid<T> & Grid<T>::operator = (Grid<T> && other) {
    if (this == &other)
        return *this;

    if(m_data != nullptr) {
        delete [] m_data;
    }

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
    if (this == &other)
        return *this;

	//-- Only allocate new memory if there is a difference in Grid sizes:
	if (m_width != other.m_width || m_height != other.m_height) {
		m_width = other.m_width;
		m_height = other.m_height;

		delete [] m_data;
		m_data = new T [m_width * m_height];
	}

    m_cellLength = other.m_cellLength;
    m_origin = other.m_origin;

    uint32 num_elements = m_width * m_height;

	// Perform deep copy of data.
    std::memcpy(m_data, other.m_data, sizeof(T)*num_elements);

    return *this;
};

//---------------------------------------------------------------------------------------
template <typename T>
void Grid<T>::setAll(const T & val) {
    for(int i(0); i < (m_height * m_width); ++i) {
        m_data[i] = val;
    }
}

//---------------------------------------------------------------------------------------
template <typename T>
const T * Grid<T>::data() const {
    return m_data;
}

//---------------------------------------------------------------------------------------
template <typename T>
glm::ivec2 Grid<T>::gridCoordOf(const glm::vec2 & p) const {
	assert(m_cellLength > 0.0f);

	vec2 relativePos = p - m_origin;
	int32 i = int32(relativePos.x / m_cellLength);
	int32 j = int32(relativePos.y / m_cellLength);

	return glm::ivec2(i,j);
}

//---------------------------------------------------------------------------------------
template <typename T>
bool Grid<T>::contains(const vec2 & p) const {
	vec2 relPos = p - m_origin;

	return (relPos.x > 0) &&
			(relPos.y > 0) &&
			(relPos.x < m_width * m_cellLength) &&
			(relPos.y < m_height * m_cellLength);
}

//---------------------------------------------------------------------------------------
template <typename T>
bool Grid<T>::isValidCoord(int32 col, int32 row) const {
	return (col > -1) &&
		   (col < m_width) &&
		   (row > -1) &&
		   (row < m_height);
}

//---------------------------------------------------------------------------------------
template <typename T>
bool Grid<T>::isValidCoord(const glm::ivec2 & gridCoord) const {
	return isValidCoord(gridCoord.x, gridCoord.y);
}

} // end namespace FluidSim
