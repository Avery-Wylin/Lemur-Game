#version 400 core


in vec3 normal_f;
in vec3 pos_f;

out vec4 color_out;

uniform vec3 cam_pos;
uniform vec3 zenith;
uniform vec3 horizon;
uniform vec3 sun;
uniform vec3 sun_dir;
uniform vec4 water;

void main(void){
vec3 n = -normalize(normal_f);
color_out.xyz = zenith + horizon * pow(1-abs(dot(n, vec3(0,1,0))),2);
float f = pow( (dot(sun_dir, n) + 1)*.5, 32);
f = f > 0.98?1:f*0.25;

color_out.rgb = color_out.xyz + clamp(f*sun,0,1);

// Water horizon
if(pos_f.y<water.w)
    color_out.rgb = water.rgb*.5*sun;

}
