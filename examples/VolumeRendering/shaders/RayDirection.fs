// RayDirection.fs
#version 410

in vec3 f_color;

out vec4 outColor;

uniform sampler2D rayEntryTexture;

// Write normalized directions (s,t,r) to RGB channels.
// Write magnitude of ray direction into Alpha channel.
void main () {
    vec3 rayStart = texture(rayEntryTexture, gl_FragCoord.xy).rgb;
    vec3 rayDirection = f_color - rayStart;

    outColor.rgb = normalize(rayDirection);
    outColor.a = length(rayDirection);


    // TODO Dustin - remove this:
        outColor.gb = vec2(0,0);
        outColor.r = length(rayDirection);
        outColor.a = 1.0;
}
