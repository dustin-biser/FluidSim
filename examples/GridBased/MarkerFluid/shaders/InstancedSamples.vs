#version 410

layout (location = 0) in vec2 vertex_position;
layout (location = 1) in vec2 particle_position;

uniform mat3 modelMatrix;
uniform mat3 worldMatrix;

void main() {
	vec3 p0 = modelMatrix * vec3(vertex_position, 1.0);
	vec3 p1 = worldMatrix * (p0 + vec3(particle_position, 0.0));

    gl_Position = vec4(p1, 1.0);
}
