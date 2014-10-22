#version 400

in vec2 f_TexCoord;

uniform sampler2D u_tex2DInk; // Opacity AlphaMap
uniform vec3 u_inkColor;

out vec4 outColor;

void main() {
    outColor = texture(u_tex2DInk, f_TexCoord).r * vec4(u_inkColor, 1.0);
}


