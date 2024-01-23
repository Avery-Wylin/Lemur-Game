#version 400 core

const int joint_count = 150;
const int weight_count = 4;

in vec3 pos;
in vec3 normal;
in vec3 vertex_color;
in vec4 weights;
in uvec4 joint_ids;

out vec3 pos_f;
out vec3 normal_f;
out vec3 vertex_color_f;
out vec3 to_camera;

uniform mat4 camera;
uniform mat4 joints[joint_count];
uniform vec3 cam_pos;

uniform mat4 transform;

void main(void){

    // Mesh Transformations
    vec4 weighted_pos = vec4(0,0,0,1);
    vec4 weighted_normal = vec4(0.0);
    vec4 test = vec4(0.0);

    for(int i = 0; i < weight_count; i++){
        if(joint_ids[i] != 0){
            weighted_pos += weights[i]*(joints[joint_ids[i]] * vec4(pos,1.0));
            weighted_normal += weights[i]*(joints[joint_ids[i]] * vec4(normal,0.0));
        }

    }

//     weighted_pos = joints[0] * vec4(pos,1);
//     weighted_normal = joints[0] * vec4(normal,0);
//     weighted_pos = transform * vec4(pos,1);
//     weighted_normal = transform * vec4(normal,0);


    // 3D Transformations
    pos_f = weighted_pos.xyz;
    gl_Position = camera * vec4(pos_f,1);
    normal_f = weighted_normal.xyz;
    vertex_color_f = vertex_color;
    to_camera = pos_f - cam_pos;

//     pos_f = (transform * vec4(pos,1)).xyz;
//     gl_Position = camera * vec4(pos_f,1);
//     vertex_color_f = vertex_color.xyz;
//     normal_f = (transform * vec4(normal,0)).xyz;
//     to_camera = pos_f - cam_pos;
}
