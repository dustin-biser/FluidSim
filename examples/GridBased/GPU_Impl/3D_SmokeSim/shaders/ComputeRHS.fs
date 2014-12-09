// ComputeRHS.fs
#version 410

// (s,t) texture-coordinates into dataGrid
in vec2 dataCoord;

out float result;

struct Grid {
    vec2 worldOrigin;
    float cellLength;
    int textureWidth;    // Given in number of cells
    int textureHeight;   // Given in number of cells
    sampler2D textureUnit; // Active texture unit
};
uniform Grid u_velocityGrid;
uniform Grid v_velocityGrid;
uniform Grid cellTypeGrid;

uniform float timeStep;
uniform float density;
uniform float u_solid;
uniform float v_solid;

bool cellIsFluid(ivec2 cellIndex) {
    vec2 texCoord = vec2(cellIndex.x / float(cellTypeGrid.textureWidth),
                         cellIndex.y / float(cellTypeGrid.textureHeight));
    return texture(cellTypeGrid.textureUnit, texCoord).r < 0.9;
}

bool cellIsSolid(ivec2 cellIndex) {
    return !cellIsFluid(cellIndex);
}

// Sample from u_velocity texture.
float u(ivec2 cellIndex) {
    vec2 texCoord = vec2(cellIndex.x / float(u_velocityGrid.textureWidth),
                         cellIndex.y / float(u_velocityGrid.textureHeight));
    return texture(u_velocityGrid.textureUnit, texCoord).r;
}

// Sample from v_velocity texture.
float v(ivec2 cellIndex) {
    vec2 texCoord = vec2(cellIndex.x / float(v_velocityGrid.textureWidth),
                         cellIndex.y / float(v_velocityGrid.textureHeight));
    return texture(v_velocityGrid.textureUnit, texCoord).r;
}

void main() {
    float scale = density * cellTypeGrid.cellLength / timeStep;

    ivec2 cellIndex = ivec2(dataCoord.s * cellTypeGrid.textureWidth,
                            dataCoord.t * cellTypeGrid.textureHeight);

    float delta_u =  u(cellIndex+ivec2(1,0)) - u(cellIndex);
    float delta_v =  v(cellIndex+ivec2(0,1)) - v(cellIndex);

        // TODO Dustin - remove this after testing:
            float garbage = u_solid * v_solid;
            scale = 1.0;
            result = texture(v_velocityGrid.textureUnit, dataCoord).r;

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

