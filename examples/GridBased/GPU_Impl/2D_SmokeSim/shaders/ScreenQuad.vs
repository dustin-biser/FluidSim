// ScreenQuad.vs
#version 410

layout (location = 0) in vec2 v_position;
layout (location = 1) in vec2 v_textureCoord;

out vec2 f_texCoord;

void main() {
    f_texCoord = v_textureCoord;
    gl_Position = vec4(v_position, 0, 1.0);
}
