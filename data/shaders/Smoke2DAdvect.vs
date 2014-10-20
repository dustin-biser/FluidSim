#version 400

layout (location = 0) in vec2 v_Position;
layout (location = 1) in vec2 v_TexCoord;

out vec2 f_TexCoord;

void main() {
    f_TexCoord = v_TexCoord;

    gl_Position = vec4(v_Position, 0, 1.0);
}
