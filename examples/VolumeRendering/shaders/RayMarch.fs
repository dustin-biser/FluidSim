// RayMarch.fs
#version 410

in vec3 f_color; // Entrance point into 3d texture space.

out float outDensity;

uniform sampler3D dataTexture3d;
uniform sampler2D rayDirection_texture2d;
uniform sampler2D noise_texture2d;

uniform float stepSize;

uniform float one_over_framebufferWidth;
uniform float one_over_framebufferHeight;


void main() {
    vec2 texCoords;
    texCoords.s = gl_FragCoord.x * float(one_over_framebufferWidth);
    texCoords.t = gl_FragCoord.y * float(one_over_framebufferHeight);

    vec4 rayInfo = texture(rayDirection_texture2d, texCoords);
    vec3 rayDir = rayInfo.rgb;
    float rayLength = rayInfo.a;
    vec3 ray;

    float noise = texture(noise_texture2d, texCoords).r;
    float step = stepSize + 0.1*noise;

    outDensity = 0;

    // March ray through bounding volume while accumulating density values:
    for(float s = 0; s < rayLength; s += step) {

        ray = f_color + (s * rayDir);
        outDensity += texture(dataTexture3d, ray).r * step;

        if(outDensity > 0.9) {
            return;
        }
    }

}