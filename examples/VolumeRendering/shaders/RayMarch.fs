// RayMarch.fs
#version 410

in vec3 f_color; // Entrance point into 3d texture space.

out float outDensity;

uniform sampler3D dataTexture3d;
uniform sampler2D rayDirection_texture2d;
uniform sampler2D accumulatedDensity_texture2d;

uniform float stepSize;

uniform uint iterationCount;
uniform float one_over_framebufferWidth;
uniform float one_over_framebufferHeight;


void main() {
    vec2 texCoords;
    texCoords.s = gl_FragCoord.x * float(one_over_framebufferWidth);
    texCoords.t = gl_FragCoord.y * float(one_over_framebufferHeight);

    vec3 rayDir = texture(rayDirection_texture2d, texCoords).rgb;
    vec3 ray = f_color;
    outDensity = texture(accumulatedDensity_texture2d, texCoords).r;

    float currentLength = iterationCount * stepSize;
    ray = f_color + currentLength * rayDir;

    outDensity += texture(dataTexture3d, ray).r * stepSize;
}