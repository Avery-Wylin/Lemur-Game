#ifndef SCENE_H
#define SCENE_H

#include <inttypes.h>

#include "View.h"
#include "VAO.h"
#include "Shader.h"
#include "Terrain.h"
#include "Sky.h"
#include "WaterPlane.h"
#include "EntitySystem.h"
#include "./scene/Player.h"
#include "physics/PhysicsSystem.h"
#include <vector>
#include "Player.h"
#include "Plant.h"

class Scene {

    Shader terrain_shader, object_shader, anim_shader, plant_shader;
    VAO terrain_vao;
    Terrain terrain;
    Sky sky;
    Water water;

public:
    PlayerSet player_set;
    PlantSystem plant_system;
    View view;
    // PlayerContainer players;
    EntitySystem entity_system;

    Scene();
    virtual ~Scene();

    void init_client(Client *client);
    void init_server(Server *server);

    void close_client();
    void close_server();
    void init_assets();
    void close_assets();
    void draw(float interp_fac);
    void update();
};
#endif // SCENE_H
