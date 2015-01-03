// RenderIsoSurface.vs
#version 410

layout (location = 0) in vec3 v_position; // vertex position.
layout (location = 1) in vec3 v_normal; // vertex normal.

uniform mat4 MVP_Matrix; // model-view-projection matrix.
uniform mat4 NormalMatrix; // transforms world-space normal to eye-space.

out v2fOutput {
	vec3 normal;
} vs_out;

void main() {
	gl_Position = MVP_Matrix * vec4(v_position, 1.0);

	vs_out.normal = (NormalMatrix * vec4(v_normal, 1.0)).xyz;
}
