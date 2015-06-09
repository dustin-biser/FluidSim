/**
 * Math.hpp
 */

#pragma once

#include <cmath>

namespace Synergy {

    constexpr float PI = 3.1415926;

    //-----------------------------------------------------------------------------------
	/**
	* Converts degrees to radians.
	*/
    inline float radians(float degrees) {
        return PI * (degrees / 180.0f);
    }

    //-----------------------------------------------------------------------------------
	/**
	* Converts radians to degrees.
	*/
    inline float degrees(float radians) {
        return 180.0f * (radians / PI);
    }

}
