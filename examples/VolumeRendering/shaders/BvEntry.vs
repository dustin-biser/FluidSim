// BvEntry.vs
#version 410

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_color;

uniform mat4 modelView_matrix;
uniform mat4 projection_matrix;

out vec3 f_color;

void main() {
    f_color = v_color;

    // Transform vertex position to eye coordinate space.
    gl_Position = projection_matrix * modelView_matrix * vec4(v_position, 1.0);
}