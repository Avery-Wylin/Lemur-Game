#ifndef WATERPLANE_H
#define WATERPLANE_H

#include "definitions.h"
#include <cglm/cglm.h>
#include "Sky.h"
#include "../graphics/VAO.h"
#include "../graphics/Shader.h"
#include "../graphics/View.h"

class Sky;

class Water {

    vec4 color_depth = {0, .5, 1, 1};
    float wave_time = 0;

public:
    static void init_assets();
    static void close_assets();

    const vec4& getUniform();
    float getWaterLevel();
    void setWaterLevel(float level);
    void draw(View &view, Sky &sky);
    void update();

};

#endif // WATERPLANE_H
