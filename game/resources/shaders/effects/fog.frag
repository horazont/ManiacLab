#version 330 core

uniform sampler2D fog_data;
uniform sampler2D fog_noise;

uniform float t;

const float flow_factor = 0.1f;

const float max_flow = 1.f;

in vec2 tc0;
in vec2 world_pos;

vec2 sample_flowdir(
        const vec2 tilecoord,
        const vec2 tileoffset)
{
    return textureLod(fog_data, (floor(tilecoord.xy + tileoffset))/(vec2(GRID_SIZE)*2.f), 0).yz * flow_factor;
}

vec2 calc_noise_texcoord(
        vec2 flowdir,
        const vec2 wavecoord,
        const vec2 timeoffset)
{
    float flowintensity = length(flowdir);
    // // invert the length of flowdir
    // flowdir /= flowintensity*flowintensity;

    /*float flowintensity = min(length(flowdir) * flow_factor, 1.f);*/

    if (flowintensity < 1.f) {
        /*flowdir = flowdir + winddir * (1-flowintensity);*/
        flowdir /= flowintensity;
    } else if (flowintensity > max_flow) {
        flowdir /= flowintensity;
        flowdir *= max_flow;
    }

    /*if (flowintensity > 1e-6) {
         flowdir = normalize(flowdir);
    } else {
        flowdir = vec2(0, 0);
        flowintensity = 0.;
    }

    flowdir = flowdir * flowintensity + winddir * (1.-flowintensity);
    flowdir = normalize(flowdir);*/

    mat2 rotmat = mat2(flowdir.x, -flowdir.y, flowdir.y, flowdir.x);
    return rotmat*wavecoord-timeoffset*flowintensity;
}

float sample_density(
        const vec2 wavecoord,
        const vec2 tilecoord,
        const vec2 tileoffset,
        const vec2 timeoffset)
{
    return textureLod(fog_noise, calc_noise_texcoord(sample_flowdir(tilecoord, tileoffset), wavecoord, timeoffset), 0).r;
}

void main() {
    vec4 fog_sample = texture(fog_data, tc0);
    float fog_density = (1-exp(-fog_sample.x)) /*  * min(pow(fog_sample.w, 1.f/2.2f), 1.f) */;
    /* fog_density = fog_density * texture(fog_noise, world_pos / 5.f).r; */
    vec2 cell_coord = tc0 * vec2(GRID_SIZE) * 2.f;
    vec2 noise_tex_base_coord = world_pos;
    vec2 ff = 1 - abs(2*fract(cell_coord)-1);
    vec2 timeoffset = vec2(100.f * t, 0.f);

    float density_A = sample_density(noise_tex_base_coord, cell_coord, vec2(0.f, 0.f), timeoffset);
    float density_B = sample_density(noise_tex_base_coord, cell_coord, vec2(0.5f, 0.f), timeoffset);
    float density_C = sample_density(noise_tex_base_coord, cell_coord, vec2(0.f, 0.5f), timeoffset);
    float density_D = sample_density(noise_tex_base_coord, cell_coord, vec2(0.5f, 0.5f), timeoffset);

    float density_AB = ff.x * density_A + (1-ff.x) * density_B;
    float density_CD = ff.x * density_C + (1-ff.x) * density_D;
    float density = ff.y * density_AB + (1-ff.y) * density_CD;

    /* gl_FragColor = vec4(vec3(fog_density) * 0.8f, fog_density); */
    float flow_intensity = max(1.f, length(fog_sample.yz) * 1000.f);
    float mixed_density = mix(1.f, density / 11.f + 0.9f, fog_density / flow_intensity) * fog_density;
    gl_FragColor = vec4(vec3(mixed_density), fog_density);
    /* gl_FragColor = vec4(abs(sample_density(world_pos / 5.f, cell_coord, vec2(0.f), vec2(t * 0.08f, 0.f))) * 100.f, 0.f, 0.5f); */
    /* gl_FragColor = vec4(abs(textureLod(fog_data, tc0, 0).yz) * 1.5f, 0.f, 0.5f); */

    /* gl_FragColor = vec4(rounded_cell_coord * 10.f, 0.f, 0.5f); */

    /* gl_FragColor = vec4(abs(sample_flowdir(cell_coord, vec2(0.f, 0.f)) * 1000.f), 0.f, 0.5f); */
    /* gl_FragColor = vec4(vec3(length(sample_flowdir(cell_coord, vec2(0.f, 0.f)) * 1000.f)), 0.5f); */

    /* gl_FragColor = vec4(
                mod(calc_noise_texcoord(sample_flowdir(cell_coord, vec2(0.f, 0.f)), world_pos / 5.f, vec2(0.08f * t, 0.f)), vec2(1.f)), 0.f, 0.5f); */

    /* gl_FragColor = vec4(vec3(sample_density(noise_tex_base_coord, cell_coord, vec2(0.f, 0.f), vec2(0.08f * t, 0.f))), 0.5f); */
}
