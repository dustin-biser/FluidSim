// ProjectV.fs
// Subtracts off the gradient of pressure from the v_velocity field in order to
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
uniform Grid v_velocityGrid;

uniform float scale;
uniform float solid_velocity;

// The current layer being processed within v_velocityGrid
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

float project_v(vec3 cellIndex) {
    // Check if top or bottom cell neighbors are solid:
    if (cellIsSolid(cellIndex) || cellIsSolid(cellIndex - vec3(0,1,0))) {
        return solid_velocity;
    }
    // Get bottom and top neighbor pressure values.
    float pB = Sample(pressureGrid, cellIndex - vec3(0,1,0));
    float pT = Sample(pressureGrid, cellIndex);

    return Sample(v_velocityGrid, cellIndex) - scale*(pT-pB);
}


void main() {
    // The current cell being processed
    vec3 cellIndex = vec3(gl_FragCoord.x, gl_FragCoord.y, currentLayer+0.5);

    result = project_v(cellIndex);
}
