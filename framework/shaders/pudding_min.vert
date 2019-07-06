#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec4 color;

uniform mat4 model_mat;
uniform mat4 view_mat;
uniform mat4 proj_mat;
uniform mat4 light_space_matrix;
uniform vec4 light_position;

out vec4 interp_color;
out vec3 interp_normal;
out vec4 unprojected_position;
out vec4 light_position_screen_space;
out vec4 position_light_space;

void main()
{
	unprojected_position = view_mat * model_mat * vec4(position.x, position.y, position.z, 1.0);
	light_position_screen_space = view_mat * model_mat * light_position;
    gl_Position = proj_mat * unprojected_position;
    interp_color = color;
	interp_normal = normalize((transpose(inverse(model_mat)) * vec4(normal, 0.0)).xyz);

	vec3 pos2 = vec3(model_mat * vec4(position, 1.0));
	position_light_space = light_space_matrix * vec4(pos2, 1.0f);
}
