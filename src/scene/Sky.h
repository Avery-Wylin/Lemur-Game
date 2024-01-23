#ifndef SKY_H
#define SKY_H

#include "../graphics/Shader.h"
#include "../graphics/VAO.h"
#include "../graphics/View.h"
#include "WaterPlane.h"

class Water;
class Sky {

public:
    vec3
    zenith = GLM_VEC3_ZERO_INIT,
    horizon = GLM_VEC3_ZERO_INIT,
    sun = GLM_VEC3_ZERO_INIT,
    sun_dir = GLM_VEC3_ZERO_INIT,

    sunset_zenith = {.2, .04, .3},
    sunset_horizon = {.3, .2, .02},
    sunset_sun = {1, .3, .1},

    noon_zenith = {.05, .2, .7},
    noon_horizon = {.1, .4, .35},
    noon_sun = {1, 1, 1},

    night_zenith = {.01, .02, .1},
    night_horizon = {.02, .02, .1},
    night_sun = {0, 0, 0};

    vec2 fog = {0.01,5};

    float t;
    static void init_assets();
    static void close_assets();

    void setSunDirection(float x, float y, float z);
    void setFog(float multiplier, float exponent){ fog[0] = multiplier; fog[1] = exponent;};
    void draw(View &view, Water &water);


};

#endif // SKY_H
