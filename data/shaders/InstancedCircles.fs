// InstancedCircles.frag
#version 400

in vec3 f_color;

layout (location = 0) out vec4 fragColor;

void main() {
   fragColor = vec4(f_color, 1);
}
