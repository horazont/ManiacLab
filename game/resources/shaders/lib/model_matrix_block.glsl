#version 330 core

layout(std140) uniform ModelMatrixBlock {
   layout(row_major) mat4 model;
   layout(row_major) mat4 model_inv;
} mats;
