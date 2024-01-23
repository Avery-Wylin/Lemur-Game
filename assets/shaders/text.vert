#version 420 core

in vec2 pos;
in vec2 uv;
in vec3 vertex_color;

out vec2 uv_f;
out vec2 pos_f;

uniform mat4 camera;
uniform mat4 transform;

void main(void){
    gl_Position = camera * transform * vec4(pos.xy, -0.1, 1.0);
    uv_f = uv;
    pos_f = pos;
}
