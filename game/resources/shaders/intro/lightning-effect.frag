#version 330 core

uniform sampler2D diffuse;
uniform float alpha;

in vec2 tc0;

void main() {
    vec4 colour = texture(diffuse, tc0).rgba;
    /* gl_FragColor = vec4(vec3(1.f), colour.a / 10.f); */

    gl_FragColor = vec4(vec3(1.f) * colour.a * alpha, 0.f);
}
