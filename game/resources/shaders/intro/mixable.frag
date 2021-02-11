#version 330 core

uniform sampler2D diffuse;
uniform vec4 mix_colour;

in vec2 tc0;

void main() {
    vec4 colour = texture(diffuse, tc0).rgba;
    colour = vec4(colour.rgb * colour.a, colour.a);
    colour = colour * (1.f - mix_colour.a) + mix_colour;
    gl_FragColor = colour;
    /* gl_FragColor = vec4(1.f, 0.f, 1.f, 1.f); */
}
