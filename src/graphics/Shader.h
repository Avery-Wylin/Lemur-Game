#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <cglm/cglm.h>
#include "definitions.h"

using std::string;

enum Attribute : uint8_t {
    ATTRB_POS,      // 3f stores local position
    ATTRB_NORM,     // 3f stores vertex normal
    ATTRB_UV,       // 2-4f stores texture coords
    ATTRB_COL,      // 3-4f stores colors
    ATTRB_WEIGHTS,  // 4f stores weights of joints
    ATTRB_JOINTS,   // 4i stores joint ids
    ATTRB_SK_POS,   // 3f stores position for shape keys
    ATTRB_SK_NORM,  // 3f stores normal for shape keys
    NUM_ATTRBS      // Last enum, number of existing attributes
};

enum Uniform : uint8_t {
    UNIFORM_TRANSFORM,   // mat4 Local transform
    UNIFORM_CAMERA,      // mat4 Camera transform (usually inverted and combined with perspective)
    UNIFORM_CAM_POS,     // vec3 Camera position (useful for incoming vector)
    UNIFORM_COLOR,       // 3f color of object
    UNIFORM_COLOR2,      // 3f color of object
    UNIFORM_FACTOR,      // f any given factor
    UNIFORM_ZENITH,      // 3f color of environment looking up
    UNIFORM_HORIZON,     // 3f color of the environment and fog
    UNIFORM_SUN,         // 3f color of the environment looking down
    UNIFORM_SUN_DIR,     // 3f color of the environment looking down
    UNIFORM_WATER,       // 4f color of the water and its depth
    UNIFORM_FOG,         // 2f fog settings multiplier & exponent
    UNIFORM_SHAPEKEY,    // f interpolation of shapekey (pos2)
    UNIFORM_JOINTS,      // mat4[] list of joint transforms
    NUM_UNIFORMS         // Last enum, number of existing uniforms
};

class Shader {

    private:

        uint32_t
        program_id,
        v_shader_id,
        f_shader_id;

        string v_shader_name, f_shader_name;
        uint32_t uniform_locations[NUM_UNIFORMS];
        string uniform_names[Uniform::NUM_UNIFORMS];
        string attrb_names[Attribute::NUM_ATTRBS];



    public:
        Shader();
        ~Shader();

        void load( string v_shader, string f_shader = "" );
        void getUniformLocations( string[] );
        void recompile();
        void linkUniform( std::string uniformName, Uniform uniform );
        void free();

        static void bind(Shader &shader);
        static void unbind();
        static void uniformMat4f( Uniform, const mat4& );
        static void uniformMat4fArray(Uniform, mat4*, uint32_t);
        static void uniformVec4f( Uniform, const vec4& );
        static void uniformVec3f( Uniform, const vec3& );
        static void uniformVec2f( Uniform, const vec2& );
        static void uniformFloat( Uniform, float );
        static void uniformInt( Uniform, int );

    private:
        void linkDefaultAttributes();
        void linkDefaultUniforms();
        void loadFromFile( string, int );


};


#endif /* SHADER_H */

