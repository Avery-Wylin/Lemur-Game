#version 400 core

in vec3 pos;
in vec3 normal;

out vec3 normal_f;
out vec3 pos_f;

uniform mat4 camera;
uniform vec3 cam_pos;
uniform vec3 perspective;
uniform vec4 water;

void main(void){
    mat4 tr =
    mat4(
    100,0,0,0,
    0,100,0,0,
    0,0,100,0,
    cam_pos.x,water.w,cam_pos.z,1);

    pos_f = (tr * ( vec4(pos,1))).xyz;
    gl_Position = camera * vec4(pos_f,1);
    normal_f = normal;
}
