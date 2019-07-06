#version 330 core

in vec4 interp_color;
in vec3 interp_normal;
in vec4 unprojected_position;
in vec4 light_position_screen_space;
in vec4 position_light_space;

uniform vec3 light_dir;
uniform sampler2D shadow_map;

out vec4 frag_color;


void main()
{
	vec4 shadow_factor;
	vec2 texelSize = 1.0 / textureSize(shadow_map, 0);

	vec3 proj_coords = position_light_space.xyz / position_light_space.w;
	proj_coords = 0.5 * proj_coords + 0.5;

	for(int x = -2; x <= 2; ++x) {
    	for(int y = -2; y <= 2; ++y) {
			float closest = texture(shadow_map, proj_coords.xy + (x, y) * texelSize).r;
			float frag_depth = proj_coords.z;
			float bias = 0.00005;
			if (closest >= frag_depth-bias) {
				shadow_factor += vec4(1.0, 1.0, 1.0, 1.0);
			} else {
				shadow_factor += vec4(0.6, 0.9, 0.6, 1.0);
			}
		}
	}
	shadow_factor /= 25;

	vec3 half = normalize(normalize(-unprojected_position.xyz / unprojected_position.a) + normalize(light_position_screen_space.xyz / light_position_screen_space.a - unprojected_position.xyz / unprojected_position.a));

    float light = dot(interp_normal, light_dir);
	float alpha = interp_color.a;
    frag_color = clamp(0.5 + pow(dot(interp_normal, half), 1), 0.5, 1.0) * interp_color * shadow_factor; // clamp(light, 0.1, 1.0) * interp_color;
	frag_color.a = alpha;
}
