// InstancedCircles.vert
#version 400

// Advanced once per vertex.
layout (location = 0) in vec2 v_Position; // Vertex position in Model Space

// Advanced once per instance.
layout (location = 1 ) in vec2 v_ParticleCenter; // World Space

uniform vec3 u_Color;
uniform mat4 u_ViewProjectionMatrix;

out vec3 f_color;

void main() {
    f_color = u_Color;

    vec4 world_pos = vec4(vec3(v_Position,0) + vec3(v_ParticleCenter,0), 1);
    gl_Position = u_ViewProjectionMatrix * world_pos;
}
