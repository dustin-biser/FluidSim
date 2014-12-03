// ScreenQuad.fs
#version 410

uniform sampler2D u_textureUnit;

in vec2 f_texCoord;

out vec4 outColor;

void main () {
    outColor = texture(u_textureUnit, f_texCoord);
}