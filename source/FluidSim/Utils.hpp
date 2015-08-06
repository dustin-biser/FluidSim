#pragma once

#include <climits>
#include <cassert>
#include <glm/glm.hpp>

namespace FluidSim {

const float32 PI = 3.14159265359f;
const float32 EPSILON = 1e-6;

//---------------------------------------------------------------------------------------
template<typename T>
void set_array (T * array, size_t size, T value) {
	assert(array);

	for (size_t i(0); i < size; ++i) {
		array[i] = value;
	}
}

//---------------------------------------------------------------------------------------
inline unsigned int randhash(unsigned int seed) {
	unsigned int i = (seed ^ 12345391u) * 2654435769u;
	i ^= (i << 6) ^ (i >> 26);
	i *= 2654435769u;
	i += (i << 5) ^ (i >> 12);

	return i;
}

//---------------------------------------------------------------------------------------
inline float randhash(float seed) {
	unsigned int i = (((unsigned int) seed) ^ 12345391u) * 2654435769u;
	i ^= (i << 6) ^ (i >> 26);
	i *= 2654435769u;
	i += (i << 5) ^ (i >> 12);

	return float(i) / float(UINT_MAX);
}

//---------------------------------------------------------------------------------------
inline glm::vec2 rand_point_in_annulus (
		const glm::vec2 & center,
		float innerRadius,
		float outerRadius,
		float seed = 0.0f
) {
	// Random radius within annulus
	float r = innerRadius + randhash(++seed) * (outerRadius - innerRadius);

	//--Compute x value:
	float sign;
	sign = float32(int32(randhash(++seed) + 0.5f)); // clamp to 0 or 1
	sign = 2.0f*sign - 1.0f;
	float x = sign * r * randhash(++seed) * 0.99999f; // x in (-r, r)

	//--Compute y value:
	sign = float32(int32(randhash(++seed) + 0.5f)); // clamp to 0 or 1
	sign = 2.0f*sign - 1.0f;
	float y = sign * float32(sqrt(r*r - x*x));

	return vec2(x + center.x, y + center.y);
}


//---------------------------------------------------------------------------------------
// Approximate, relative, absolute tolerance floating point check.
template <typename T,
		  typename = std::enable_if<std::is_floating_point<T>::value> >
bool approxEqual(T x, T y) {
	return std::abs(x - y) <= T(EPSILON) * (abs(x) + abs(y) + T(1));
}

} // end namespace FluidSim
