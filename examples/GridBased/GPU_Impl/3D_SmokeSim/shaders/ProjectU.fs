// ProjectU.fs
// Subtracts off the gradient of pressure from the u_velocity field in order to
// make it divergence free.
#version 410

out float result;

struct Grid {
    int textureWidth;      // Given in number of cells
    int textureHeight;     // Given in number of cells
    int textureDepth;      // Given in number of cells
    sampler3D textureUnit; // Active texture unit
};
uniform Grid pressureGrid;
uniform Grid cellTypeGrid;
uniform Grid u_velocityGrid;

uniform float scale;
uniform float solid_velocity;

// The current layer being processed within u_velocityGrid
uniform uint currentLayer;


float Sample(in Grid grid, in vec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(grid.textureWidth),
                         cellIndex.y / float(grid.textureHeight),
                         cellIndex.z / float(grid.textureDepth));
    return texture(grid.textureUnit, texCoord).r;
}

bool cellIsSolid(in vec3 cellIndex) {
    return Sample(cellTypeGrid, cellIndex) > 0.9;
}


float project_u(vec3 cellIndex) {
    // Check if right or left cell neighbors are solid:
    if (cellIsSolid(cellIndex) || cellIsSolid(cellIndex - vec3(1,0,0))) {
        return solid_velocity;
    }
    // Get left and right neighbor pressure values.
    float pL = Sample(pressureGrid, cellIndex - vec3(1,0,0));
    float pR = Sample(pressureGrid, cellIndex);

    return Sample(u_velocityGrid, cellIndex) - scale*(pR-pL);
}


void main() {
    // The current cell being processed
    vec3 cellIndex = vec3(gl_FragCoord.x, gl_FragCoord.y, currentLayer+0.5);

    result = project_u(cellIndex);
}

//----------------------------------------------------------------------------------------
// TESTS TO CHECK
//----------------------------------------------------------------------------------------
// 1. PASS - Check that solid/fluid cells are correct using cellIndex
// 2. PASS - correct values from project_u(...).
