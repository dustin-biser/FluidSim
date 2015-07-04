#pragma once

//-- OpenGL:
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#elif defined __linux
    #include <GL/gl.h>
#endif

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Synergy {

//-- Basic Numeric Types:
typedef signed char byte;
typedef unsigned char ubyte;

typedef signed char	int8;
typedef signed short int16;
typedef signed int int32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

typedef float float32;
typedef double float64;


//-- GLM types:
typedef glm::vec2 vec2;
typedef glm::vec3 vec3;
typedef glm::vec4 vec4;

typedef glm::mat2 mat2;
typedef glm::mat3 mat3;
typedef glm::mat4 mat4;

typedef glm::quat quat;

}
