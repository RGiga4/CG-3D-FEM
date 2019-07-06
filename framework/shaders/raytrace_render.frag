#version 330 core

in vec4 interp_color;
in vec3 interp_normal;
in vec3 interp_position;
uniform vec3 light_pos;

out vec4 frag_color;

void main()
{
    float light = dot(interp_normal, normalize(light_pos - interp_position));
    frag_color = clamp(light, 0.1, 1.0) * interp_color;
}
