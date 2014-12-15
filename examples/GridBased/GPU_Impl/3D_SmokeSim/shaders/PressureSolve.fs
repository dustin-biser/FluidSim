// PressureSolve.fs
#version 410

// (s,t) texture-coordinate
in vec2 f_textureCoord;

out float result;

struct Grid {
    int textureWidth;      // Given in number of cells
    int textureHeight;     // Given in number of cells
    int textureDepth;      // Given in number of cells
    sampler3D textureUnit; // Active texture unit
};
uniform Grid pressureGrid;
uniform Grid divergenceGrid;
uniform Grid cellTypeGrid;

// The current layer being processed within pressureGrid
uniform uint currentLayer;


float Sample(in Grid grid, in vec3 cellIndex) {
    vec3 texCoord = vec3(cellIndex.x / float(grid.textureWidth),
                         cellIndex.y / float(grid.textureHeight),
                         cellIndex.z / float(grid.textureDepth));
    return texture(grid.textureUnit, texCoord).r;
}

bool cellIsFluid(in vec3 cellIndex) {
    return Sample(cellTypeGrid, cellIndex) < 0.9;
}

bool cellIsSolid(in vec3 cellIndex) {
    return !cellIsFluid(cellIndex);
}


void main() {

    // The current cell being processed
    vec3 cellIndex = vec3(gl_FragCoord.x, gl_FragCoord.y, currentLayer+0.5);

    const vec3 one_s = vec3(1,0,0);
    const vec3 one_t = vec3(0,1,0);
    const vec3 one_r = vec3(0,0,1);

    // Get pressure and divergence at the current cell:
    float pC = Sample(pressureGrid, cellIndex);
    float dC = Sample(divergenceGrid, cellIndex);

    // Get pressure value for all neighboring cells:
    float pL = Sample(pressureGrid, cellIndex - one_s); // Left neighbor
    float pR = Sample(pressureGrid, cellIndex + one_s); // Right neighbor
    float pB = Sample(pressureGrid, cellIndex - one_t); // Bottom neighbor
    float pT = Sample(pressureGrid, cellIndex + one_t); // Top neighbor
    float pN = Sample(pressureGrid, cellIndex - one_r); // Near neighbor
    float pF = Sample(pressureGrid, cellIndex + one_r); // Far neighbor

    if(cellIsSolid(cellIndex - one_s)) pL = pC;
    if(cellIsSolid(cellIndex + one_s)) pR = pC;
    if(cellIsSolid(cellIndex - one_t)) pB = pC;
    if(cellIsSolid(cellIndex + one_t)) pT = pC;
    if(cellIsSolid(cellIndex - one_r)) pN = pC;
    if(cellIsSolid(cellIndex + one_r)) pF = pC;


    result = (pL + pR + pB + pT + pN + pF - dC) / 6.0;

}
