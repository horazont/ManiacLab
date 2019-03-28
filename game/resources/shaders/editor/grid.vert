#version 330 core

{% include ":/shaders/lib/matrix_block.glsl" %}

in vec2 position;

void main()
{
    gl_Position = mats.proj * mats.view * vec4(position, 0.f, 1.f);
}
