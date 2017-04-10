#version 330 core

vec4 unproject(vec3 window_space)
{
    vec3 ndc = vec3(2*window_space.xy / inv_mats.viewport.xy - 1, 0);
    ndc.z = (2*window_space.z - gl_DepthRange.far - gl_DepthRange.near)
            / (gl_DepthRange.far - gl_DepthRange.near);

    float clip_w = mats.proj[3][2] / (ndc.z - mats.proj[2][2]/mats.proj[2][3]);
    vec4 clip = vec4(ndc * clip_w, clip_w);

    return inv_mats.proj * clip;
}
