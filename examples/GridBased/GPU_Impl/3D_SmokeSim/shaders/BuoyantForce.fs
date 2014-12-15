// BuoyantForce.fs
#version 410

// (s,t) texture-coordinate
in vec2 f_textureCoord;

out float result;

struct Grid {
    vec3 worldOrigin;
    float cellLength;      // Length of each cell
    int textureWidth;      // Given in number of texels
    int textureHeight;     // Given in number of texels
    int textureDepth;      // Given in number of texels
    sampler3D textureUnit; // Active texture unit
};
uniform Grid w_velocityGrid;
uniform Grid densityGrid;
uniform Grid temperatureGrid;

uniform float Kd;    // Density coefficient
uniform float Kt;    // Temperature coefficient
uniform float Temp0; // Ambient temperature

uniform float timeStep;
uniform uint currentLayer; // R texture layer currently being  processed.


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
    textureCoord.z = (currentLayer+0.5) / w_velocityGrid.textureDepth;

    vec3 worldPosition = getWorldPosition(w_velocityGrid, textureCoord);

    //-- Get density and temperature values at the current w_velocity cell
    float density = linearInterp(densityGrid, worldPosition);
    float temp = linearInterp(temperatureGrid, worldPosition);
    float velocity = texture(w_velocityGrid.textureUnit, textureCoord).r;

    float force = -Kd * density + (Kt * (temp - Temp0));

    result = velocity + timeStep*force;
}
