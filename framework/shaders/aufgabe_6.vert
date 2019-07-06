#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;

out vec3 interp_pos;

void main()
{
    gl_Position = proj_mat * view_mat * model_mat * vec4(position.x, position.y, position.z, 1.0);
    interp_pos = normalize(position.xyz);
}
