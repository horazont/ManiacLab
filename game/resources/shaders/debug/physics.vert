#version 330 core

{% include ":/shaders/lib/matrix_block.glsl" %}

uniform sampler2D debug_tex;

in vec2 tc0;
in vec3 position;

out vec2 texcoord;


void main()
{
    texcoord = tc0;
    vec3 position = position.xyz;
    gl_Position = mats.proj * mats.view * vec4(position, 1.f);
}
