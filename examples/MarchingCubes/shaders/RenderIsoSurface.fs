// RenderIsoSurface.fs
#version 410

uniform vec3 color;

in v2fOutput {
	vec3 normal;
} fs_in;

out vec4 out_color;


void main() {
	// Attenuate surface color based on the dot product of surface normal and view
	// direction (along -z).
	vec3 minus_z_dir = vec3(0,0,-1);
	float n_dot_minus_z = max(dot(fs_in.normal, minus_z_dir), 0.0);

	out_color = vec4(color * n_dot_minus_z, 1.0);
}

