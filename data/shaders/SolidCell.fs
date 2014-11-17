#version 400

uniform vec3 u_solidCellColor;

out vec4 outColor;

void main() {
    outColor = vec4(u_solidCellColor, 1.0);
}