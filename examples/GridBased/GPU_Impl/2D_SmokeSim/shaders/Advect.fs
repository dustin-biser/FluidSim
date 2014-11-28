// Advect.fs
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
uniform Grid u_velocityGrid; // Horizontal Velocity
uniform Grid v_velocityGrid; // Vertical Velocity
uniform Grid dataGrid;       // Data to be advected

uniform float timeStep;

// Converts the texture coordinates 'texCoord' into a world position given the
// worldOrigin and cellLength of 'grid'.
vec2 getWorldPosition(in Grid grid, in vec2 texCoord) {
    texCoord *= grid.cellLength;
    return grid.worldOrigin + texCoord;
}

vec2 getTextureCoords(in Grid grid, in vec2 worldPos) {
    vec2 result = worldPos - grid.worldOrigin;
    float dx = grid.cellLength;
    return result / vec2(dx, dx);
}

float linearInterp(in Grid grid, in vec2 worldPosition) {
    vec2 texCoords = getTextureCoords(grid, worldPosition);
    return texture(grid.textureUnit, texCoords).r;
}


void main () {
    // Compute world position of 'dataCoord' within 'dataGrid'.
    vec2 worldPosition = getWorldPosition(dataGrid, dataCoord);

    // Interpolate (u,v) velocity at this world location.
    float u = linearInterp(u_velocityGrid, worldPosition);
    float v = linearInterp(v_velocityGrid, worldPosition);
    vec2 velocity = vec2(u,v);

    // Backtrace to particle location that will end up at 'worldPosition' at the
    // next timeStep.
    // Two stage Runge-Kutta:
    vec2 midPoint = worldPosition - (0.5 * timeStep * velocity);
    u = linearInterp(u_velocityGrid, midPoint);
    v = linearInterp(v_velocityGrid, midPoint);
    velocity = vec2(u,v);
    worldPosition = worldPosition - (timeStep * velocity);

    result = linearInterp(dataGrid, worldPosition);

}

