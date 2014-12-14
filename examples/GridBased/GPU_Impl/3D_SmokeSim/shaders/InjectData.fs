// InjectData.fs
#version 410

out float result;

struct Grid {
    int textureWidth;      // Given in number of texels
    int textureHeight;     // Given in number of texels
    int textureDepth;      // Given in number of texels
    sampler3D textureUnit; // Active texture unit
};
uniform Grid dataGrid;

uniform float currentLayer;
uniform float value;

void main() {
    vec3 textureCoord;
    textureCoord.x = gl_FragCoord.x / float(dataGrid.textureWidth);
    textureCoord.y = gl_FragCoord.y / float(dataGrid.textureHeight);
    textureCoord.z = (currentLayer+0.5) / dataGrid.textureDepth;

    result = texture(dataGrid.textureUnit, textureCoord).r + value;
}