#version 330 core

{% include ":/shaders/lib/matrix_block.glsl" %}

in vec3 position;


void main()
{
    vec3 position = position.xyz;
    gl_Position = mats.proj * mats.view * vec4(position, 1.f);
}
