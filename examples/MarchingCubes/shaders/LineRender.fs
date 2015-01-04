// LineRender.fs
#version 410

out vec4 outColor;

uniform vec3 lineColor;

void main() {
    outColor = vec4(lineColor, 1.0);
}
