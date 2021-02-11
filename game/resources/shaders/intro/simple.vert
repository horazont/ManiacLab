#version 330 core

{% include ":/shaders/lib/matrix_block.glsl" %}

in vec4 position;
in vec2 tc0_in;

out vec2 tc0;
out vec2 world_pos;

void main()
{
    tc0 = tc0_in;
    world_pos = position.xy;
    /* gl_Position = vec4(vec3(mats.proj * mats.view * vec4(position.xyz * position.w, 1.f)), position.w); */
    gl_Position = mats.proj * mats.view * position;
}
