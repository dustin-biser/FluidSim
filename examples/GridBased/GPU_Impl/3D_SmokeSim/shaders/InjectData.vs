// InjectData.vs
#version 410

layout (location = 0) in vec2 v_position;

uniform mat4 modelMatrix;

void main() {
    gl_Position = modelMatrix * vec4(v_position, 0, 1.0);
}
