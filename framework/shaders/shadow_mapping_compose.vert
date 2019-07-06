#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;

out vec4 interp_color;
out vec3 interp_normal;

out vec4 pos_light_space;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform mat4 light_space_matrix;

void main()
{
	gl_Position = proj_mat * view_mat * model_mat * vec4(position, 1);
	interp_color = color;
	interp_normal = transpose(inverse(mat3(model_mat))) * normal;

	vec3 pos2 = vec3(model_mat * vec4(position, 1.0));
	pos_light_space = light_space_matrix * vec4(pos2, 1.0f);
}
