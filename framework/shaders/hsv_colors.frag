#version 330 core

in vec4 interp_color;

out vec4 frag_color;

vec4 hsva2rgba(vec4 hsva) {
    float i = hsva[0]*6;
	vec4 res = vec4(0.0, 0.0, 0.0, hsva[3]);

	float c = hsva[1]*hsva[2]; //S*V
	float x = mod(i, 2.0)-1;
	if(x<0)x = -x; //betrag
	x = c*(1-x); //1-|i mod 2 -1|

	float m = hsva[2]-c;//V-C

	if(i>=0 && i<=6){
		     if(i<=1)res = vec4(c, x, 0.0, hsva[3]);
		else if(i<=2)res = vec4(x, c, 0.0, hsva[3]);
		else if(i<=3)res = vec4(0.0, c, x, hsva[3]);
		else if(i<=4)res = vec4(0.0, x, c, hsva[3]);
		else if(i<=5)res = vec4(x, 0.0, c, hsva[3]);
		else if(i<=6)res = vec4(c, 0.0, x, hsva[3]);
	}else vec4(0.0, 0.0, 0.0, hsva[3]);

	res[0] = res[0]+m;
	res[1] = res[1]+m;
	res[2] = res[2]+m;

    return res;
}

void main()
{
    frag_color = hsva2rgba(interp_color);
}
