// RayDirection.fs
#version 410

in vec3 f_color;

out vec4 outColor;

uniform sampler2D rayEntryTexture;
uniform int framebufferWidth;
uniform int framebufferHeight;

// Write normalized directions (s,t,r) to RGB channels.
// Write magnitude of ray direction into Alpha channel.
void main () {
    vec2 texCoords;
    texCoords.s = gl_FragCoord.x / float(framebufferWidth);
    texCoords.t = gl_FragCoord.y / float(framebufferHeight);

    vec3 rayStart = texture(rayEntryTexture, texCoords).rgb;
    vec3 rayDirection = f_color - rayStart;

    outColor.rgb = normalize(rayDirection);
    outColor.a = length(rayDirection);
}

