// RayStoppingCriteria.fs
#version 410

in vec3 f_color;

uniform sampler2D rayDirection_texture2d;
uniform sampler2D accumulatedDensity_texture2d;
uniform float stepSize;
uniform uint iterationCount;
uniform float maxDensityThreshold;
uniform float one_over_framebufferWidth;
uniform float one_over_framebufferHeight;

// Check if density threshold has been reached or if current ray length is greater than
// ray distance through bounding volume.  If true, set gl_FragDepth = 1.0 which will
// cause early fragment discard due to depth testing.
void main() {
    vec2 texCoords;
    texCoords.s = gl_FragCoord.x * float(one_over_framebufferWidth);
    texCoords.t = gl_FragCoord.y * float(one_over_framebufferHeight);

    float currentDensity = texture(accumulatedDensity_texture2d, texCoords).r;

    if (currentDensity < maxDensityThreshold) {
        vec4 rayDir = texture(rayDirection_texture2d, texCoords);
        float maxRayDist = rayDir.a;
        float s = stepSize * iterationCount;
        vec3 ray = f_color + (s * rayDir.rgb);

        if (length(ray) >= maxRayDist) {
            // Force early depth test fragment discard.
            gl_FragDepth = 1.0;
        }

    } else {
            // Force early depth test fragment discard.
            gl_FragDepth = 1.0;
    }

}
