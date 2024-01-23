#include "Sky.h"
#include "WaterPlane.h"

static Shader sky_shader;
static VAO sky_vao;


void Sky::init_assets(){
    sky_vao.loadPLY("sky");
    sky_shader.load("sky");
}

void Sky::close_assets(){
    sky_vao.free();
    sky_shader.free();
}

void Sky::setSunDirection(float x, float y, float z){
    vec3 up = {0,1,0};
    sun_dir[0] = x;
    sun_dir[1] = y;
    sun_dir[2] = z;
    glm_vec3_normalize(sun_dir);
    // Use the angle of the sun to determine the time of day
    float day_time = (glm_vec3_dot(sun_dir,up) + 1) * .5f;

    if(day_time > 0.5){ // Sunrise/set to Noon
        // Convert half-time into 0-1
        day_time = 2*(day_time - 0.5);

        glm_vec3_lerp(sunset_zenith, noon_zenith, day_time, zenith);
        glm_vec3_lerp(sunset_horizon, noon_horizon, day_time, horizon);
        glm_vec3_lerp(sunset_sun, noon_sun, day_time, sun);
    }
    else{ // Midnight to Sunrise/Set
        // Convert half-time into 0-1, then bias it towards night
        day_time = pow ( 2*day_time, 8);

        glm_vec3_lerp(night_zenith ,sunset_zenith, day_time, zenith);
        glm_vec3_lerp(night_horizon ,sunset_horizon, day_time, horizon);
        glm_vec3_lerp(night_sun ,sunset_sun, day_time, sun);
    }
}

void Sky::draw(View &view, Water &water){

    Shader::bind(sky_shader);
    Shader::uniformMat4f(UNIFORM_CAMERA, view.getPerspective());
    Shader::uniformVec3f(UNIFORM_CAM_POS, view.pos);
    Shader::uniformMat4f(UNIFORM_TRANSFORM, view.getInverseTransform());
    Shader::uniformVec3f(UNIFORM_ZENITH, zenith);
    Shader::uniformVec3f(UNIFORM_HORIZON, horizon);
    Shader::uniformVec3f(UNIFORM_SUN, sun);
    Shader::uniformVec3f(UNIFORM_SUN_DIR, sun_dir);
    Shader::uniformVec4f(UNIFORM_WATER, water.getUniform());

    sky_vao.bind();
    glDrawElements(GL_TRIANGLES, sky_vao.getIndexCount(), GL_UNSIGNED_INT, 0);
}
