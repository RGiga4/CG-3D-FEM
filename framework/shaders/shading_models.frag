#version 330 core

in vec4 interp_color;
in vec3 interp_normal;
in vec3 interp_light_dir;

out vec4 frag_color;

uniform bool useOrenNayar;

uniform float roughness; // sigma
uniform float refractionIndex;
uniform vec4 diffuse; // diffuse part as color
uniform vec4 specular; // specular part as color

const float pi = 3.14159265359;

// Syntatic sugar. Make sure dot products only map to hemisphere
float cdot(vec3 a, vec3 b) {
    return clamp(dot(a,b), 0.0, 1.0);
}

// D
float beckmannDistribution(float dotNH) {
	// cos(alpha) is dotNH; m is roughtness
    float sigma2 = roughness * roughness;
    // TASK: Compute d-term
	float t = 1/(pow(dotNH,4)*sigma2);
	
    return t*exp(-pow(tan(acos(dotNH))/roughness, 2));
}

// F
float schlickApprox(float dotVH, float n1, float n2) {
    // TASK: Compute f-term nach Wikipedia
	float R0 = (n1-n2)/(n1+n2);
	R0 = R0*R0;
    float t = R0 + (1.0-R0)*pow(1.0 - dotVH ,5.0);
	return t;
}

// G
float geometricAttenuation(float dotNH, float dotVN, float dotVH, float dotNL) {
    // TASK: Compute g-term
	float t = min(dotVN, dotNL);
	
    return min(1.0, 2*dotNH*t/dotVH);
}

float cooktorranceTerm(vec3 n, vec3 l) {
    vec3 v = vec3(0.0, 0.0, 1.0); // in eye space direction towards viewer simply is the Z axis
    vec3 h = normalize(l + v); // half-vector between V and L

    // precompute to avoid redundant computation
    float dotVN = cdot(v, n);
    float dotNL = cdot(n, l);
    float dotNH = cdot(n, h);
    float dotVH = cdot(v, h);

    float D = beckmannDistribution(dotNH);
    float F = schlickApprox(dotVH, 1.0, refractionIndex);
    float G = geometricAttenuation(dotNH, dotVN, dotVH, dotNL);

    return max(D * F * G / (4.0 * dotVN * dotNL), 0.0);
}

float orennayarTerm(float lambert, vec3 n, vec3 l) {
    vec3 v = vec3(0.0, 0.0, 1.0); // Im eye space ist die Richtung zum Betrachter schlicht die Z-Achse
    float sigma2 = roughness * roughness; // sigma^2

    // TASK: implement Oren-Nayar Term and use instead of 1.0 below:

	float A = 1 -0.5*sigma2/(sigma2+0.33);
	float B = 0.45*sigma2/(sigma2+0.09);

	vec3 v_t = normalize(v-dot(v,n)*n);
	vec3 l_t = normalize(l-dot(l,n)*n);
	float cos_term = cdot(l_t, v_t);

	float tha_v = acos(dot(n,v));
	float tha_l = acos(dot(n,l));

	float alpha = max(tha_v, tha_l);
	float beta  = min(tha_v, tha_l);


    return lambert * (A+B*max(0, cos_term)*sin(alpha)*tan(beta));
}

void main() {
    // Lambertian reflection term
    float diffuseTerm = cdot(interp_normal, interp_light_dir);


    // define the diffuse part to be Lambertian - unless we choose Oren-Nayer
    // in this case compute Oren-Nayar reusing the Lambertian term and use that

    if (useOrenNayar) {
        diffuseTerm = orennayarTerm(diffuseTerm, interp_normal, interp_light_dir);
    }

    // lowest possbile value = ambient fake light term
    diffuseTerm = max(diffuseTerm, 0.1);


    // as specular part we compute the Cook-Torrance term
    float specularTerm = cooktorranceTerm(interp_normal, interp_light_dir);

    // combine both terms (diffuse+specular) using our material properties (colors)
    frag_color = vec4(vec3(clamp(diffuse * diffuseTerm + specular * specularTerm, 0.0, 1.0)), 1);
}
