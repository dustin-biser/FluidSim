// ProjectW.fs
// Subtracts off the gradient of pressure from the w_velocity field in order to
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
uniform Grid w_velocityGrid;

uniform float scale;
uniform float solid_velocity;

// The current layer being processed within w_velocityGrid
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


float project_w(vec3 cellIndex) {
    // Check if far or near cell neighbors are solid:
    if (cellIsSolid(cellIndex) || cellIsSolid(cellIndex - vec3(0,0,1))) {
        return solid_velocity;
    }
    // Get near and far neighbor pressure values.
    float pN = Sample(pressureGrid, cellIndex - vec3(0,0,1));
    float pF = Sample(pressureGrid, cellIndex);

    return Sample(w_velocityGrid, cellIndex) - scale*(pF-pN);
}


void main() {

    // The current cell being processed
    vec3 cellIndex = vec3(gl_FragCoord.x, gl_FragCoord.y, currentLayer+0.5);

    result = project_w(cellIndex);
}