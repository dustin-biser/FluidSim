// ScreenQuad.fs
#version 410

uniform sampler2D u_textureUnit;

// (s,t) texture-coordinate
in vec2 f_textureCoord;

out vec4 outColor;

void main () {
    outColor = texture(u_textureUnit, f_textureCoord);
}