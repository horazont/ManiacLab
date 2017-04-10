#version 330 core

layout(std140) uniform InvMatrixBlock {
   layout(row_major) mat4 proj;
   layout(row_major) mat4 view;
   vec2 viewport;
} inv_mats;
