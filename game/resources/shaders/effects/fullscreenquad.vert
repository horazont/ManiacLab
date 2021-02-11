#version 330 core

uniform vec2 fetch_offset;
uniform vec2 fetch_scale;

in vec2 position;
out vec2 coord;
out vec2 fetch_coord;

void main()
{
    gl_Position = vec4(position, 0, 1);
    vec2 coord_tmp = position / 2 + 0.5;
    coord = coord_tmp;
    fetch_coord = coord_tmp * fetch_scale;
}
