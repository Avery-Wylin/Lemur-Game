#version 420 core

in vec3 uv_f;
in vec3 pos_f;
in vec3 vertex_color_f;
in vec3 normal_f;
in vec3 to_camera;

uniform vec3 zenith;
uniform vec3 horizon;
uniform vec3 sun;
uniform vec3 sun_dir;
uniform vec4 water;
uniform vec2 fog_data;

out vec4 color_out;

void main(void){

    // calculate normal
    vec3 n = normalize(normal_f);
    float f = max(dot(sun_dir, n),0);

    // blend color
    color_out.xyz = (zenith + horizon * pow(1-abs(dot(n, vec3(0,1,0))),.5) + f * sun) * vertex_color_f;

    // Water depth factor
    f = .25*(water.w-pos_f.y);

    // subtract color based on depth
    color_out.rgb *= pos_f.y < water.w ? min( max(water.rgb + vec3(.9) - f,water.xyz-1 ),vec3(1)) : vec3(1);

    // calculate fog factor
    vec2 fog = pos_f.y < water.w ? vec2(0.1, 1):fog_data;
    float fog_factor = min( pow(length(to_camera.xz) * fog.x,fog.y ) ,1);

    // blend fog
    color_out.rgb = mix( color_out.rgb, (pos_f.y < water.w ? water.rgb*.5*sun: zenith+horizon), fog_factor);
}
