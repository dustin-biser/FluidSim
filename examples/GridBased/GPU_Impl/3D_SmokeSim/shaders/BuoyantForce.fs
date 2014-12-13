// BuoyantForce.fs
#version 410

// (s,t) texture-coordinate
in vec2 f_textureCoord;

out float result;

struct Grid {
    vec3 worldOrigin;
    float cellLength;      // Length of each cell
    sampler3D textureUnit; // Active texture unit
};
uniform Grid w_velocityGrid;
uniform Grid densityGrid;
uniform Grid temperatureGrid;

uniform float Kd;    // Density coefficient
uniform float Kt;    // Temperature coefficient
uniform float Temp0; // Ambient temperature

uniform float timeStep;
uniform float w_velocityDepth;
uniform uint currentLayer; // R texture layer currently being  processed.


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
    // Normalized texture coordinate within w_velocityGrid
    vec3 texCoord = vec3(f_textureCoord, currentLayer / w_velocityDepth);

    // Compute world position of 'dataCoord' within 'dataGrid'.
    vec3 worldPosition = getWorldPosition(w_velocityGrid, texCoord);

    //-- Get density and temperature values at the current w_velocity cell
    float density = linearInterp(densityGrid, worldPosition);
    float temp = linearInterp(temperatureGrid, worldPosition);

    float force = -Kd * density + (Kt * (temp - Temp0));

    float w_vel = linearInterp(w_velocityGrid, worldPosition);
    result = w_vel + timeStep*force;
}
