#version 410

layout (location = 0) in vec2 position;

uniform mat3 modelMatrix;

void main() {
	vec3 p = modelMatrix * vec3(position, 1.0f);
    gl_Position = vec4(p, 1.0);
}
