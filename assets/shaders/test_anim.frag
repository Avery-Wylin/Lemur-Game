#version 420 core

in vec3 pos_f;
in vec3 vertex_color_f;
in vec3 normal_f;
in vec3 to_camera;

uniform vec3 cam_pos;
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

    // Ambient and diffuse color
    color_out.xyz = (zenith + horizon * pow(1-max(dot(n, vec3(0,1,0)),0),2) + f * sun) * vertex_color_f;

    // Water depth overlay
    f = .25*(water.w-pos_f.y);
    color_out.rgb *= pos_f.y < water.w ? min( max(water.rgb + .9 - f,water.xyz-1 ),vec3(1)) : vec3(1);

    // Determine if camera and position are above water
    float cam_ab_w = float( cam_pos.y > water.w);
    float pos_ab_w = float( pos_f.y > water.w);

    // Fake volumetric for water by using depth of ray passing through water
    vec3 water_depth = to_camera;

    // Calculate plane intersect
    float t = min((water.w - cam_pos.y)/(pos_f.y-cam_pos.y),1);

    // Adjust depth read based on location
    t = pos_ab_w - t*( - cam_ab_w + pos_ab_w);

    // Vector that passes through water
    water_depth = (pos_f*t + cam_pos*(1-t)) - pos_f;

    // Water fog falloff
    float water_fog = min(0.05*length(water_depth),1);

    // Air fog overlay
    float fog_factor = min( pow(length(to_camera) * fog_data.x,fog_data.y ) ,1);
    color_out.rgb = mix( color_out.rgb, zenith+horizon, fog_factor);

    // Water fog overlay
    color_out.rgb = mix( color_out.rgb, water.rgb * sun * .5, water_fog);

}

