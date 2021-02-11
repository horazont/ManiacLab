#version 330 core

uniform vec2 position;
uniform vec2 scale;
uniform vec3 edge_colour;

in vec2 coord;

void main() {
    vec2 final_coord = (coord - 0.5f) * 2.f;
    final_coord = final_coord * scale;
    final_coord = final_coord + position;
    /* final_coord = final_coord / 2.f + 0.5f; */
    float distance = 1.f - cos(min(length(final_coord), 1.f) * 3.14159 / 2.f);
    gl_FragColor = vec4(edge_colour * distance, distance);
}
