#ifndef BEACHBALL_H
#define BEACHBALL_H

#include "../graphics/VAO.h"
#include "EntitySystem.h"

struct BeachBall{

};

class TypeBeachBall : public EntityType {
    static VAO vao;
    std::vector<BeachBall> instances;

public:
    TypeBeachBall();
    ~TypeBeachBall();
    void init_assets();
    void close_assets();

    void update();
    void draw();
    void create();
    void remove();
};

#endif // BEACHBALL_H
