#version 400 core

/* https://www.shadertoy.com/view/XsX3zB
 *
 * The MIT License
 * Copyright © 2013 Nikita Miropolskiy
 *
 * ( license has been changed from CCA-NC-SA 3.0 to MIT
 *
 *   but thanks for attributing your source code when deriving from this sample
 *   with a following link: https://www.shadertoy.com/view/XsX3zB )
 */

/* discontinuous pseudorandom uniformly distributed in [-0.5, +0.5]^3 */
vec3 random3(vec3 c) {
	float j = 4096.0*sin(dot(c,vec3(17.0, 59.4, 15.0)));
	vec3 r;
	r.z = fract(512.0*j);
	j *= .125;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}

/* skew constants for 3d simplex functions */
const float F3 =  0.3333333;
const float G3 =  0.1666667;

/* 3d simplex noise */
float simplex3d(vec3 p) {
	 /* 1. find current tetrahedron T and it's four vertices */
	 /* s, s+i1, s+i2, s+1.0 - absolute skewed (integer) coordinates of T vertices */
	 /* x, x1, x2, x3 - unskewed coordinates of p relative to each of T vertices*/

	 /* calculate s and x */
	 vec3 s = floor(p + dot(p, vec3(F3)));
	 vec3 x = p - s + dot(s, vec3(G3));

	 /* calculate i1 and i2 */
	 vec3 e = step(vec3(0.0), x - x.yzx);
	 vec3 i1 = e*(1.0 - e.zxy);
	 vec3 i2 = 1.0 - e.zxy*(1.0 - e);

	 /* x1, x2, x3 */
	 vec3 x1 = x - i1 + G3;
	 vec3 x2 = x - i2 + 2.0*G3;
	 vec3 x3 = x - 1.0 + 3.0*G3;

	 /* 2. find four surflets and store them in d */
	 vec4 w, d;

	 /* calculate surflet weights */
	 w.x = dot(x, x);
	 w.y = dot(x1, x1);
	 w.z = dot(x2, x2);
	 w.w = dot(x3, x3);

	 /* w fades from 0.6 at the center of the surflet to 0.0 at the margin */
	 w = max(0.6 - w, 0.0);

	 /* calculate surflet components */
	 d.x = dot(random3(s), x);
	 d.y = dot(random3(s + i1), x1);
	 d.z = dot(random3(s + i2), x2);
	 d.w = dot(random3(s + 1.0), x3);

	 /* multiply d by w^4 */
	 w *= w;
	 w *= w;
	 d *= w;

	 /* 3. return the sum of the four surflets */
	 return dot(d, vec4(52.0));
}



in vec3 normal_f;
in vec3 pos_f;

out vec4 color_out;

uniform vec3 cam_pos;
uniform vec3 horizon;
uniform vec3 zenith;
uniform vec3 sun;
uniform vec3 sun_dir;
uniform float factor;
uniform vec2 fog_data;
uniform vec4 water;

void main(void){

// Calculate wave normal
vec3 n = vec3 (
        simplex3d( vec3(pos_f.x+.03,pos_f.z,factor)),
        simplex3d( vec3(pos_f.x,pos_f.z,factor)),
        simplex3d( vec3(pos_f.x,pos_f.z+.03,factor))
    );
n.x = n.y-n.x;
n.z = n.y-n.z;
// Larger Y values smooth out the waves
n.y=.7;
n = normalize(n);
n *= gl_FrontFacing?1:-1;
vec3 incoming = normalize(cam_pos - pos_f);

// Half-angle with sun direction for specularity
vec3 half_angle = normalize(incoming + sun_dir);
// Create sun specularity on front-facing
if(gl_FrontFacing && pow(max(dot(n,half_angle),0)*.75,4) > .3){
	color_out.rgb = sun;
	return;
}

// Water fresnel
float fresnel = pow ((1 - abs( dot(n,incoming) )) * .8, 4);
if(fresnel < .35)
	discard;
vec3 fog_col =  water.rgb*.2*sun + .6*(zenith+horizon);
color_out.rgb = fog_col;
}


