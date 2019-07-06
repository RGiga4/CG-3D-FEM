#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform bool show_edge_wrap;

out vec2 interp_uv;

void main()
{
    gl_Position = proj_mat * view_mat * model_mat * vec4(position.x, position.y, position.z, 1.0);

    // vertices in original triangle are in [-1,1] for X and Z coordinate, we can compute
    // UV coordinates by simply transforming these linearly into [0,1]
    // if factor == 1.0 we instead scale the UV coordinates to [0,2] resulting in an edge wrap for half the geometry
    // (along each axis).
    float factor = show_edge_wrap ? 1.0 : 0.5;
    interp_uv = factor * (position.xy + vec2(1.0));
}
