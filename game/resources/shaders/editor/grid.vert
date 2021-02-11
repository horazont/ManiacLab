#version 330 core

{% include ":/shaders/lib/matrix_block.glsl" %}

in vec2 position;

uniform float intensity;

void main()
{
    gl_Position = mats.proj * mats.view * vec4(position, (9 + intensity), 1.f);
}
