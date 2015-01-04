// LineRender.vs
#version 410

layout (location = 0) in vec3 v_position;

// Model-View-Projection matrix.
uniform mat4 MVP_Matrix;

void main() {
    // Transform vertex position to eye coordinate space.
    gl_Position = MVP_Matrix * vec4(v_position, 1.0);
}
