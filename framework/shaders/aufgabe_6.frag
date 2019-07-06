#version 330 core

in vec3 interp_pos;

out vec4 frag_color;

uniform sampler2D tex;
uniform bool cylindrical;

const float eps = 1E-06;
const float pi = 3.14159265359;

vec2 getUVCoordinatesCyl(vec3 p) {
    // TASK:
    // Implement the determination of suitable UV
    // coordinates to perform a cylindrical projection
    // of the texture onto the sphere's surface.
	
	float u = atan(p.y , p.x)/(2*pi) + 0.5;// mir sind die Falluntescheidungen und Sonderfalle bekannt
	float v = p.z * 0.5 + 0.5;
    vec2 uv = vec2(u, v);

    // ...

    return uv;
}

vec2 getUVCoordinatesSph(vec3 p) {
    // TASK:
    // Implement the determination of suitable UV
    // coordinates to perform a spherical projection
    // of the texture onto the sphere's surface.

	float u = atan(p.y , p.x)/(2*pi) + 0.5;
	float v = asin(p.z)/(2*pi) + 0.5;
    vec2 uv = vec2(u, v);

    // ...

    return uv;
}

void main() {
    vec2 tc;

    // Decide whether to use cylindrical or spherical
    // mapping depending on what is chosen by the user.
    if (cylindrical) {
        tc = getUVCoordinatesCyl(normalize(interp_pos));
    } else {
        tc = getUVCoordinatesSph(normalize(interp_pos));
    }

    frag_color = texture2D(tex, tc);
}
