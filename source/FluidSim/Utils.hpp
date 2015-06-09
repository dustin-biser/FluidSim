#pragma once

#include <climits>
#include <cassert>
#include <glm/glm.hpp>

namespace FluidSim {

namespace Utils {

static const float32 PI = 3.14159265359f;

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
// Returns the length squared of argument.
template <typename T>
T length2 (
		const glm::detail::tvec2<T, glm::highp> & vec
) {
	return (vec.x * vec.x) + (vec.y * vec.y);
}


} // end namespace Utils

} // end namespace FluidSim
