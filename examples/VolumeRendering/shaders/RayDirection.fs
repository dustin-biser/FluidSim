// RayDirection.fs
#version 410

in vec3 f_color;

out vec4 outColor;

uniform sampler2D rayEntryTexture;
uniform float one_over_framebufferWidth;
uniform float one_over_framebufferHeight;

// Write normalized directions (s,t,r) to outColor.rgb
// Write magnitude of ray direction outColor.a
void main () {
    vec2 texCoords;
    texCoords.s = gl_FragCoord.x * one_over_framebufferWidth;
    texCoords.t = gl_FragCoord.y * one_over_framebufferHeight;

    vec3 rayStart = texture(rayEntryTexture, texCoords).rgb;
    vec3 rayDirection = f_color - rayStart;

    outColor.rgb = normalize(rayDirection);
    outColor.a = length(rayDirection);
}

