#version 330 core

uniform float texture_scale;
uniform sampler2D diffuse;

in vec2 world_pos;

void main() {
    gl_FragColor = texture(diffuse, world_pos * texture_scale).rgba;
}
