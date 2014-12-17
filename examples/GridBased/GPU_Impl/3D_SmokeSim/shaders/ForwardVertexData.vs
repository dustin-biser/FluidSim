// ForwardPositions.vs
#version 410

layout (location = 0) in vec2 v_position;
layout (location = 1) in vec2 v_textureCoord;

// (s,t) texture-coordinate
out vec2 f_textureCoord;

void main() {
    f_textureCoord = v_textureCoord;

    gl_Position = vec4(v_position, 0, 1.0);
}
