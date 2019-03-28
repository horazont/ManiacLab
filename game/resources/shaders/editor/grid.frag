#version 330 core

out vec4 colour;

uniform float intensity;

void main() {
    colour = vec4(vec3(intensity), 0.9);
}
