#version 400

layout (location = 0) in vec2 v_VertexOffset;
layout (location = 2) in vec2 v_CenterPosition;

uniform float u_cellLength;

void main() {

    //-- Map grid space to NDC space:
    // First map vertexOffset to gridspace (aka, worldspace)
    vec2 vertexOffset_WS = u_cellLength*0.5*v_VertexOffset; //world space
    // Then map world space to NDC space
    vec2 position = 2.0*(v_CenterPosition+vertexOffset_WS);
    position -= vec2(1,1);

    gl_Position = vec4(position, 0.0, 1.0);
}