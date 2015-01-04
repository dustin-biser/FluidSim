// RenderIsoSurface.fs
#version 410

uniform vec3 color;

in v2fOutput {
	vec3 normal;
} fs_in;

out vec4 out_color;


void main() {
	// Attenuate surface color based on the dot product of surface normal and view
	// direction.
	vec3 z_dir = vec3(0,0,1);
	float n_dot_z = dot(fs_in.normal, z_dir);

	vec3 fragColor = color;

	// Flip the color on back facing polygons:
	if (n_dot_z < 0) {
		n_dot_z *= -1.0;
		fragColor = vec3(1) - color;
	}

	out_color = vec4(fragColor * n_dot_z, 1.0);
}

