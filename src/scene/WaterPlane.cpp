#include "WaterPlane.h"
#include "Armature.h"


static Shader water_shader;
static VAO plane;


void Water::init_assets(){
    water_shader.load("water");
    float pos[12] = { -1,0,-1, -1,0,1, 1,0,1, 1,0,-1};
    plane.loadAttributeFloat(ATTRB_POS,0,0,3,sizeof(pos),pos);
}

void Water::close_assets(){
    water_shader.free();
    plane.free();
}

float Water::getWaterLevel(){
    return color_depth[3];
}

void Water::setWaterLevel(float level){
    color_depth[3] = level;
    bouyancy_height = level;
}

const vec4& Water::getUniform(){
    return color_depth;
}

void Water::draw(View &view, Sky &sky){
    glDisable(GL_CULL_FACE);
    Shader::bind(water_shader);
    Shader::uniformMat4f(UNIFORM_CAMERA, view.getCombinedTransform());
    Shader::uniformVec3f(UNIFORM_CAM_POS, view.pos);
    Shader::uniformVec4f(UNIFORM_WATER, color_depth);
    Shader::uniformVec3f(UNIFORM_HORIZON, sky.horizon);
    Shader::uniformVec3f(UNIFORM_ZENITH, sky.zenith);
    Shader::uniformVec3f(UNIFORM_SUN, sky.sun);
    Shader::uniformVec3f(UNIFORM_SUN_DIR, sky.sun_dir);
    Shader::uniformVec2f(UNIFORM_FOG, sky.fog);
    Shader::uniformFloat(UNIFORM_FACTOR, abs(wave_time-WATER_WAVE_MOD*.5f));

    plane.bind();
    glDrawArrays(GL_QUADS, 0, 4);

    glEnable(GL_CULL_FACE);
}

void Water::update(){
    wave_time = fmod( wave_time + WATER_WAVE_SPEED, WATER_WAVE_MOD);
}
