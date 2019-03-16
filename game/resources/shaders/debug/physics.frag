#version 330 core

uniform sampler2D debug_tex;

in vec2 texcoord;

out vec4 colour;

vec3 cubehelix(
    const float gray,
    const float s,
    const float r,
    const float h)
{
    float a = h*gray*(1.f-gray)/2.f;
    float phi = 2.*3.14159*(s/3. + r*gray);

    float cos_phi = cos(phi);
    float sin_phi = sin(phi);

    float rf = clamp(
        gray + a*(-0.14861*cos_phi + 1.78277*sin_phi),
        0.0, 1.0);
    float gf = clamp(
        gray + a*(-0.29227*cos_phi - 0.90649*sin_phi),
        0.0, 1.0);
    float bf = clamp(
        gray + a*(1.97294*cos_phi),
        0.0, 1.0);

    return vec3(rf, gf, bf);
}

void main() {
    vec4 data = texture2D(debug_tex, texcoord);
    /* if (data.w > 0.5) {
        colour = vec4(0, 0, 0, 1);
    } else { */
        float temp_input = (data.z / data.x - 0.5) / 1;
        float press_input = (data.w > 0.5 ? 0.5 : data.x);
        colour = vec4(cubehelix(temp_input, 3.14159/12, -1, press_input), 1);
        colour = mix(colour, vec4(vec3(0.8), 1), min(data.y / 2, 0.9));
    /* } */

    /*if (data.w > 0.5) {
        colour = vec4(0, 1, 0, 1);
    } else {*/
        /*colour = vec4(vec3(data.z/2 / data.x) + vec3(0, 0, data.w), 1);*/
        /* colour = vec4(vec3(data.y) + vec3(0, 0, data.w), 1); */
        /* colour = vec4(vec3(data.x / 2) + vec3(0, 0, data.w), 1); */
    /*}*/
    //colour = vec4(abs(data.z), abs(data.w), 0, 1);
}
