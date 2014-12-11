// ComputeRHS.fs
#version 410

// (s,t) texture-coordinates into dataGrid
in vec2 f_textureCoord;

out float result;

struct Grid {
    float cellLength;
    int textureWidth;      // Given in number of cells
    int textureHeight;     // Given in number of cells
    int textureDepth;      // Given in number of cells
    sampler3D textureUnit; // Active texture unit
};
uniform Grid u_velocityGrid;
uniform Grid v_velocityGrid;
uniform Grid w_velocityGrid;
uniform Grid cellTypeGrid;

// The current layer being processed with cellTypeGrid.
uniform uint currentLayer;

uniform float timeStep;
uniform float density;
uniform float u_solid;
uniform float v_solid;
uniform float w_solid;

bool cellIsFluid(ivec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(cellTypeGrid.textureWidth),
                         cellIndex.y / float(cellTypeGrid.textureHeight),
                         cellIndex.z / float(cellTypeGrid.textureDepth));
    return texture(cellTypeGrid.textureUnit, texCoord).r < 0.9;
}

bool cellIsSolid(ivec3 cellIndex) {
    return !cellIsFluid(cellIndex);
}

// Sample from u_velocity texture.
float u(ivec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(u_velocityGrid.textureWidth),
                         cellIndex.y / float(u_velocityGrid.textureHeight),
                         cellIndex.z / float(u_velocityGrid.textureDepth));
    return texture(u_velocityGrid.textureUnit, texCoord).r;
}

// Sample from v_velocity texture.
float v(ivec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(v_velocityGrid.textureWidth),
                         cellIndex.y / float(v_velocityGrid.textureHeight),
                         cellIndex.z / float(v_velocityGrid.textureDepth));
    return texture(v_velocityGrid.textureUnit, texCoord).r;
}

// Sample from w_velocity texture.
float w(ivec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(w_velocityGrid.textureWidth),
                         cellIndex.y / float(w_velocityGrid.textureHeight),
                         cellIndex.z / float(w_velocityGrid.textureDepth));
    return texture(w_velocityGrid.textureUnit, texCoord).r;
}

void main() {
    float scale = density * cellTypeGrid.cellLength / timeStep;

    ivec3 cellIndex = ivec3(f_textureCoord.s * cellTypeGrid.textureWidth,
                            f_textureCoord.t * cellTypeGrid.textureHeight,
                            currentLayer * cellTypeGrid.textureDepth);

    float delta_u =  u(cellIndex+ivec3(1,0,0)) - u(cellIndex);
    float delta_v =  v(cellIndex+ivec3(0,1,0)) - v(cellIndex);

        // TODO Dustin - remove this after testing:
            float garbage = u_solid * v_solid * w_solid * w_velocityGrid.cellLength;
            scale = 1.0;
            result = scale;//texture(v_velocityGrid.textureUnit, f_textureCoord).r;

//    result =  scale * (delta_u + delta_v);

//    //-- Update RHS based on solid boundaries:
//    // Left Neighbor
//    if ( cellIsSolid(cellIndex - ivec2(1,0)) ) {
//        result += scale * (u(cellIndex) - u_solid);
//    }
//    // Right Neighbor
//    if ( cellIsSolid(cellIndex + ivec2(1,0)) ) {
//        result -= scale * (u(cellIndex + ivec2(1,0)) - u_solid);
//    }
//    // Bottom Neighbor
//    if ( cellIsSolid(cellIndex - ivec2(0,1)) ) {
//        result += scale * (v(cellIndex) - v_solid);
//    }
//    // Top Neighbor
//    if ( cellIsSolid(cellIndex + ivec2(0,1)) ) {
//        result -= scale * (v(cellIndex + ivec2(0,1)) - v_solid);
//    }

}

// TESTS:
// 1. PASSED, cellIndex calculation matches grid dimensions 512x512
// 2. PASSED, cellIsFluid()

// 3. v(cellIndex) values match input data.

