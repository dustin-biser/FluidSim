// NoiseGenerator.fs
#version 410

in vec2 f_texCoord;

out float outColor;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898, 78.233))) * 43758.5453);
}

void main () {
    outColor = rand(gl_FragCoord.xy);
}
