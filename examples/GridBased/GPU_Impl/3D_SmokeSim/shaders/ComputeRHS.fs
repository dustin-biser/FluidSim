// ComputeRHS.fs
#version 410

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

bool cellIsFluid(vec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(cellTypeGrid.textureWidth),
                         cellIndex.y / float(cellTypeGrid.textureHeight),
                         cellIndex.z / float(cellTypeGrid.textureDepth));
    return texture(cellTypeGrid.textureUnit, texCoord).r < 0.9;
}

bool cellIsSolid(vec3 cellIndex) {
    return !cellIsFluid(cellIndex);
}

// Sample from u_velocity texture.
float u(vec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(u_velocityGrid.textureWidth),
                         cellIndex.y / float(u_velocityGrid.textureHeight),
                         cellIndex.z / float(u_velocityGrid.textureDepth));
    return texture(u_velocityGrid.textureUnit, texCoord).r;
}

// Sample from v_velocity texture.
float v(vec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(v_velocityGrid.textureWidth),
                         cellIndex.y / float(v_velocityGrid.textureHeight),
                         cellIndex.z / float(v_velocityGrid.textureDepth));
    return texture(v_velocityGrid.textureUnit, texCoord).r;
}

// Sample from w_velocity texture.
float w(vec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(w_velocityGrid.textureWidth),
                         cellIndex.y / float(w_velocityGrid.textureHeight),
                         cellIndex.z / float(w_velocityGrid.textureDepth));
    return texture(w_velocityGrid.textureUnit, texCoord).r;
}

void main() {
    float scale = density * cellTypeGrid.cellLength / timeStep;

    vec3 cellIndex = vec3(gl_FragCoord.x, gl_FragCoord.y, currentLayer+0.5);

    float delta_u =  u(cellIndex+vec3(1,0,0)) - u(cellIndex);
    float delta_v =  v(cellIndex+vec3(0,1,0)) - v(cellIndex);
    float delta_w =  w(cellIndex+vec3(0,0,1)) - w(cellIndex);

    result =  scale * (delta_u + delta_v + delta_w);

    //-- Update RHS based on solid boundaries:
    // -X Neighbor Cell
    if ( cellIsSolid(cellIndex - vec3(1,0,0)) ) {
        result += scale * (u(cellIndex) - u_solid);
    }
    // +X Neighbor Cell
    if ( cellIsSolid(cellIndex + vec3(1,0,0)) ) {
        result -= scale * (u(cellIndex + vec3(1,0,0)) - u_solid);
    }
    // -Y Neighbor Cell
    if ( cellIsSolid(cellIndex - vec3(0,1,0)) ) {
        result += scale * (v(cellIndex) - v_solid);
    }
    // +Y Neighbor Cell
    if ( cellIsSolid(cellIndex + vec3(0,1,0)) ) {
        result -= scale * (v(cellIndex + vec3(0,1,0)) - v_solid);
    }
    // -Z Neighbor Cell
    if ( cellIsSolid(cellIndex - vec3(0,0,1)) ) {
        result += scale * (w(cellIndex) - w_solid);
    }
    // +Z Neighbor Cell
    if ( cellIsSolid(cellIndex + vec3(0,0,1)) ) {
        result -= scale * (w(cellIndex + vec3(0,0,1)) - w_solid);
    }

}

// TESTS
// 1. PASSED - Check that cellIndex.xyz goes from [0-127] in all directions.
// 2. PASSED - Check cellIsSolid gives correct results given cellIndex.
// 3. PASSED - delta_u
// 4. PASSED - delta_v
// 5. PASSED - delta_w

