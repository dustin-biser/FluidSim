// Advect.vs
#version 410

layout (location = 0) in vec2 v_position;
layout (location = 1) in vec2 v_textureCoord;

// (s,t,r) texture-coordinates into dataGrid.
out vec3 dataCoord;

uniform uint dataGridLevel; // Level within dataGrid that is currently being processed.

void main() {
    dataCoord.st = v_textureCoord;
    dataCoord.r = dataGridLevel;

    gl_Position = vec4(v_position, 0, 1.0);
}
