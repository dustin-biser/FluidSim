// Advect.fs
#version 410

// (s,t) texture-coordinate
in vec2 f_textureCoord;

out float result;

struct Grid {
    vec3 worldOrigin;
    float cellLength;      // Length of each cell
    int textureWidth;
    int textureHeight;
    int textureDepth;
    sampler3D textureUnit; // Active texture unit
};
uniform Grid u_velocityGrid; // u velocity component
uniform Grid v_velocityGrid; // v velocity component
uniform Grid w_velocityGrid; // w vertical component
uniform Grid dataGrid;       // Data to be advected

uniform float timeStep;
uniform uint currentLayer; // Layer within dataGrid that is currently being  processed.

// Converts the texture coordinates 'texCoord' into a world position.
vec3 getWorldPosition(in Grid grid, in vec3 texCoord) {
    // Offset due to values being at center of texture cells.
    vec3 texOffset = vec3(0.5/grid.textureWidth,
                          0.5/grid.textureHeight,
                          0.5/grid.textureDepth);
    // Move from center of texel to lower left of texel.
    texCoord -= texOffset;

    texCoord *= vec3(grid.textureWidth, grid.textureHeight, grid.textureDepth);
    texCoord *= grid.cellLength;

    return grid.worldOrigin + texCoord;
}

vec3 getTextureCoords(in Grid grid, in vec3 worldPosition) {
    vec3 texCoord = worldPosition - grid.worldOrigin;
    texCoord /= grid.cellLength;
    texCoord /= vec3(grid.textureWidth, grid.textureHeight, grid.textureDepth);

    // Offset due to values being at center of texture cells.
    vec3 texOffset = vec3(0.5/grid.textureWidth,
                          0.5/grid.textureHeight,
                          0.5/grid.textureDepth);

    return texCoord + texOffset;
}

float linearInterp(in Grid grid, in vec3 worldPosition) {
    vec3 texCoords = getTextureCoords(grid, worldPosition);
    return texture(grid.textureUnit, texCoords).r;
}

void main () {
    // Normalized texture coordinate within w_velocityGrid
    vec3 textureCoord;
    textureCoord.xy = f_textureCoord;
    textureCoord.z = (currentLayer+0.5) / dataGrid.textureDepth;

    vec3 worldPosition = getWorldPosition(dataGrid, textureCoord);

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

