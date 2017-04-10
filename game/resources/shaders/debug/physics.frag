#version 330 core

uniform sampler2D debug_tex;

in vec2 texcoord;

out vec4 colour;

void main() {
    vec4 data = texture2D(debug_tex, texcoord);
    if (data.w > 0.5) {
        colour = vec4(0, 1, 0, 1);
    } else {
        colour = vec4(vec3(data.z/data.x), 1);
    }
}
