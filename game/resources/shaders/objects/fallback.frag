#version 330 core

in vec2 world_pos;

const vec4 c1 = vec4(1.f, 1.f, 0.2f, 1.f);
const vec4 c2 = vec4(0.2f, 0.2f, 0.2f, 1.f);

void main() {
    float t = mod(world_pos.x * 2.5f + world_pos.y * 2.5, 1.f);
    vec4 c = t > 0.5 ? c1 : c2;

    gl_FragColor = c;
}
