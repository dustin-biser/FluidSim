// Advect.fs
#version 410

// (s,t,r) texture-coordinates into dataGrid
out vec3 dataCoord;

out float result;

struct Grid {
    vec3 worldOrigin;
    float cellLength;      // Length of each cell
    sampler3D textureUnit; // Active texture unit
};
uniform Grid u_velocityGrid; // u velocity component
uniform Grid v_velocityGrid; // v velocity component
uniform Grid w_velocityGrid; // w vertical component
uniform Grid dataGrid;       // Data to be advected

uniform float timeStep;

// Converts the texture coordinates 'texCoord' into a world position given the
// worldOrigin and cellLength of 'grid'.
vec3 getWorldPosition(in Grid grid, in vec3 texCoord) {
    texCoord *= grid.cellLength;
    return grid.worldOrigin + texCoord;
}

vec3 getTextureCoords(in Grid grid, in vec3 worldPosition) {
    return (worldPosition - grid.worldOrigin) / vec3(grid.cellLength);
}

float linearInterp(in Grid grid, in vec3 worldPosition) {
    vec3 texCoords = getTextureCoords(grid, worldPosition);
    return texture(grid.textureUnit, texCoords).r;
}


void main () {
    // Compute world position of 'dataCoord' within 'dataGrid'.
    vec3 worldPosition = getWorldPosition(dataGrid, dataCoord);

    // Interpolate (u,v) velocity at this world location.
    float u = linearInterp(u_velocityGrid, worldPosition);
    float v = linearInterp(v_velocityGrid, worldPosition);
    float w = linearInterp(w_velocityGrid, worldPosition);
    vec3 velocity = vec3(u,v,w);

    // Backtrace to particle location that will end up at 'worldPosition' at the
    // next timeStep.
    // Two stage Runge-Kutta:
    vec3 midPoint = worldPosition - (0.5 * timeStep * velocity);
    u = linearInterp(u_velocityGrid, midPoint);
    v = linearInterp(v_velocityGrid, midPoint);
    w = linearInterp(w_velocityGrid, midPoint);
    velocity = vec3(u,v,w);
    worldPosition = worldPosition - (timeStep * velocity);

    result = linearInterp(dataGrid, worldPosition);
}

