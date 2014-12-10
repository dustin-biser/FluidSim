// LineRender.vs
#version 410

layout (location = 0) in vec3 v_position;

uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;

void main() {
    // Transform vertex position to eye coordinate space.
    gl_Position = ProjectionMatrix * ModelViewMatrix * vec4(v_position, 1.0);
}
