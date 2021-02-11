#version 330 core

uniform sampler2D diffuse;

in vec2 tc0;

void main() {
    vec4 colour = texture(diffuse, tc0).rgba;
    gl_FragColor = vec4(colour.rgb * colour.a, colour.a);
    /* gl_FragColor = vec4(1.f, 0.f, 1.f, 1.f); */
}
