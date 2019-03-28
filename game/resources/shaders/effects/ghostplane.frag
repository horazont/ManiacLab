#version 330 core

#define BASE_AMPLIFY 1.3f

uniform sampler2D scene_colour;
uniform sampler1D gauss;
uniform float scale;
uniform float mix_factor;
uniform float distort_t;

in vec2 coord;

vec3 rgbToHsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsvToRgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 preprocess(vec3 colour)
{
#ifdef DIRECTION_H
    vec3 hsv = rgbToHsv(colour);
    hsv.y = 0.2f;
    hsv.x = 0.58f;
    vec3 result = hsvToRgb(hsv);
    result = pow(result * BASE_AMPLIFY, vec3(2.2f));
    return result;
#else
    return colour;
#endif
}

vec2 distort_lookup(float t)
{
#ifdef DIRECTION_H
    vec2 dir = vec2(1.f, 0.f);
#else
    vec2 dir = vec2(0.f, 1.f);
#endif
    return dir * t + sin(distort_t + cos(coord) * 30.f - cos(coord + distort_t / 10.f) / 4.f) * 0.05f;
}

void main()
{
    vec3 colour = vec3(0.f);
    for (int i = 0; i < BLUR_SIZE; ++i) {
        float t = float(i - BLUR_SIZE / 2) / (BLUR_SIZE / 2);
        float g = float(i) / (BLUR_SIZE - 1);
        vec2 read_offset = distort_lookup(t);
        colour += preprocess(texture(scene_colour, coord + read_offset * scale).rgb) * texture(gauss, g).r * BLUR_AMPLIFY;
    }
    /* we use an unfiltered fetch here to avoid ugly artifacts */
    vec3 orig = texelFetch(scene_colour, ivec2(coord * textureSize(scene_colour, 0)), 0).rgb;
    gl_FragColor = vec4(colour * mix_factor + orig * (1.f - mix_factor), 1);
    /* gl_FragColor = vec4(texture(scene_colour, coord + distort_lookup(0.f) * scale).rgb, 1.f); */
}
