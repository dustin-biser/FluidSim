#version 410

out vec4 fragColor;

uniform vec3 primativeColor;

void main() {
    fragColor = vec4(primativeColor, 0.0);
}
