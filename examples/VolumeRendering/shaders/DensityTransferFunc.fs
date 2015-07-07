// DensityTransferFunc.fs
#version 410

uniform sampler2D u_textureUnit;

in vec2 f_texCoord;

out vec4 outColor;

void main () {
    float density = texture(u_textureUnit, f_texCoord).r;

    if (density > 0.001) {
		vec3 colorA = vec3(0.04,0.0,0.08);
		vec3 colorB = vec3(0.5,0.5,1.0);
		vec3 amount = vec3(clamp(2*density,0.0,1.0));

		outColor = vec4(mix(colorA*0.8,colorB*5.5,amount), 1.0);
		gl_FragDepth = 0.9;

	} else {
        outColor = vec4(0.0);
		gl_FragDepth = 1.0;
	}

}
