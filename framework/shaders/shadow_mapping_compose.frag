#version 330 core

in vec4 interp_color;
in vec3 interp_normal;

in vec4 pos_light_space;

out vec4 frag_color;

uniform sampler2D shadowMap;

void main() {
	vec3 light_direction = normalize(vec3(-2.0f, 4.0f, -1.0f));

	vec3 proj_coords = pos_light_space.xyz / pos_light_space.w;
	proj_coords = 0.5 * proj_coords + 0.5;
	float closest = texture(shadowMap, proj_coords.xy).r;
	float frag_depth = proj_coords.z;
	float bias = max(0.0005 * (1.0 - dot(interp_normal, light_direction)), 0.00005);
	if (closest >= frag_depth-bias) {
		frag_color = clamp(dot(interp_normal, light_direction), 0.1, 1.0) * interp_color;
	} else {
		frag_color = 0.1 * interp_color;
	}
}