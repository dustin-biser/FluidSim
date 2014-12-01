// StencilFluidCells.fs
#version 410

uniform sampler2D u_textureUnit;

in vec2 f_texCoord;

out float outColor;

void main () {
    outColor = texture(u_textureUnit, f_texCoord).r;

    // Fluid cells = 0.0
    // Solid cells = 1.0

    // Discard all fragments that correspond to Fluid.
    // Passing fragments will pass the stencil test, and write 0 to it.
    if (outColor < 0.9) {
        discard;
    }

}