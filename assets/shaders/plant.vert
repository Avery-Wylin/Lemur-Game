#version 420 core

in vec3 pos;
in vec3 uv;
in vec4 vertex_color;
in vec3 normal;

out vec3 uv_f;
out vec3 pos_f;
out vec3 vertex_color_f;
out vec3 normal_f;
out vec3 to_camera;

uniform mat4 transform;
uniform mat4 camera;
uniform vec3 cam_pos;

// Replace these with uniforms
vec3 color_main = vec3(0.15, 0.1, 0.05);
vec3 color_r = vec3(1, 0, 0);
vec3 color_g = vec3(1, 1, 0);
vec3 color_b = vec3(0,.2,0);

void main(void){
    pos_f = (transform * vec4(pos,1)).xyz;
    gl_Position = camera * vec4(pos_f,1);
    uv_f = uv;
    vertex_color_f = color_main;
    vertex_color_f = mix( vertex_color_f, color_r, vertex_color.r);
    vertex_color_f = mix( vertex_color_f, color_g, vertex_color.g);
    vertex_color_f = mix( vertex_color_f, color_b, vertex_color.b);
    normal_f = (transform * vec4(normal,0)).xyz;
    to_camera = pos_f - cam_pos;
}
