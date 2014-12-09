// Advect.vs
#version 410

layout (location = 0) in vec2 v_position;
layout (location = 1) in vec2 v_textureCoord;

// (s,t) texture-coordinates into dataGrid
out vec2 dataCoord;

// Simple pass-through vertex shader program.
void main() {
    dataCoord = v_textureCoord;
    gl_Position = vec4(v_position, 0, 1.0);
}
