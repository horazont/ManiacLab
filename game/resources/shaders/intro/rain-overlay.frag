#version 330 core

uniform sampler2D diffuse;
uniform float scale;

in vec2 tc0;

void main() {
    float density = texture(diffuse, tc0 * scale).r;
    gl_FragColor = vec4(vec3(0.9f, 0.91f, 0.92f) * density, density);
}
