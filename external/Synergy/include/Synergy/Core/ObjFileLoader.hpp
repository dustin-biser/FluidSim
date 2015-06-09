/**
* ObjFileLoader
*/

#pragma once

#include <Synergy/Core/Settings.hpp>
#include <vector>

namespace Synergy {

class ObjFileLoader {
public:
    static void decode(const char * objFilePath,
                       std::vector<vec3> & positions,
                       std::vector<vec3> & normals,
                       std::vector<vec2> & uvCoords);

    static void decode(const char * objFilePath,
                       std::vector<vec3> & positions,
                       std::vector<vec3> & normals);

};

}

