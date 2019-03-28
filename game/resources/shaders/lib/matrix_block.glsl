#version 330 core

layout(std140) uniform MatrixBlock {
   layout(row_major) mat4 proj;
   layout(row_major) mat4 view;
   vec3 world_viewpoint;
   vec4 sun_colour;
   vec3 sun_direction;
   vec4 sky_diffuse[9];
} mats;
