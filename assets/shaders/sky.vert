#version 400 core

in vec3 pos;
in vec3 normal;

out vec3 normal_f;
out vec3 pos_f;

uniform mat4 camera;
uniform mat4 transform;
uniform vec3 cam_pos;

void main(void){
    
    // 3D Transformations
//     vec4 worldSpace = transform*vec4(pos,1.0);
//     mat4 c = camera;
//     c[3] = vec4(0,0,0,1);
//     vec4 cameraSpace = c*worldSpace;
//     gl_Position = perspective*cameraSpace;
    mat4 c = transform;
    c[3] = vec4(0,0,0,1);

    // Note that the constant scale is the frustum far distance in the definitions.h
    gl_Position = camera * c * ( vec4(80* pos,1));
    pos_f = 100*pos + cam_pos;
    normal_f = normal;

}
