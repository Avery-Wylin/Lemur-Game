#include "Scene.h"

#include <iostream>

Scene::Scene(){

}

Scene::~Scene(){

}

void Scene::draw(float interp_fac){

    // View Mode
    Player *active_player = player_set.get_active();
    Armature *active_armature = player_set.get_active_armature();
    if(active_player && active_armature){
        vec3 forward = {0,0,-2};
        vec3 up = {0,1,0};
        glm_quat_rotatev(view.rot, forward, forward);
        glm_quat_for(forward, up, active_player->look_rot);
        glm_vec3_sub(active_armature->get_transform_buffer()[0][3], forward, forward);
        glm_vec3_copy(forward,view.pos);
        view.update();
    }

    // Sky
    sky.draw(view, water);

    // Objects

    // Terrain
    mat4 ident = GLM_MAT4_IDENTITY_INIT;
    Shader::bind(terrain_shader);
    Shader::uniformMat4f(UNIFORM_TRANSFORM, ident);
    Shader::uniformMat4f(UNIFORM_CAMERA, view.getCombinedTransform());
    Shader::uniformVec3f(UNIFORM_CAM_POS, view.pos);
    Shader::uniformVec3f(UNIFORM_ZENITH, sky.zenith);
    Shader::uniformVec3f(UNIFORM_HORIZON, sky.horizon);
    Shader::uniformVec3f(UNIFORM_SUN, sky.sun);
    Shader::uniformVec3f(UNIFORM_SUN_DIR, sky.sun_dir);
    Shader::uniformVec4f(UNIFORM_WATER, water.getUniform());
    Shader::uniformVec2f(UNIFORM_FOG, sky.fog);
    terrain_vao.bind();
    glDrawElements(GL_TRIANGLES, terrain_vao.getIndexCount(), GL_UNSIGNED_INT, 0);

    // Draw players
    Shader::bind( anim_shader );
    Shader::uniformMat4f(UNIFORM_CAMERA, view.getCombinedTransform());
    Shader::uniformVec3f(UNIFORM_CAM_POS, view.pos);
    Shader::uniformVec3f(UNIFORM_ZENITH, sky.zenith);
    Shader::uniformVec3f(UNIFORM_HORIZON, sky.horizon);
    Shader::uniformVec3f(UNIFORM_SUN, sky.sun);
    Shader::uniformVec3f(UNIFORM_SUN_DIR, sky.sun_dir);
    Shader::uniformVec4f(UNIFORM_WATER, water.getUniform());
    Shader::uniformVec2f(UNIFORM_FOG, sky.fog);
    player_set.draw(interp_fac);

    // Draw plants
    glDisable(GL_CULL_FACE);
    Shader::bind( plant_shader );
    Shader::uniformMat4f(UNIFORM_CAMERA, view.getCombinedTransform());
    Shader::uniformVec3f(UNIFORM_CAM_POS, view.pos);
    Shader::uniformVec3f(UNIFORM_ZENITH, sky.zenith);
    Shader::uniformVec3f(UNIFORM_HORIZON, sky.horizon);
    Shader::uniformVec3f(UNIFORM_SUN, sky.sun);
    Shader::uniformVec3f(UNIFORM_SUN_DIR, sky.sun_dir);
    Shader::uniformVec4f(UNIFORM_WATER, water.getUniform());
    Shader::uniformVec2f(UNIFORM_FOG, sky.fog);
    plant_system.draw(view);
    glEnable(GL_CULL_FACE);

    // Water Plane
    water.draw(view,sky);

    entity_system.draw();
}

void Scene::update(){

    // Sky day/night cycle
    // sky.t += 0.01f;
    // sky.setSunDirection(cos(sky.t),sin(sky.t),0);

    /* NOTE the proposed physics order for objects:
     * 1. Update based on state queries and input
     * 2. Correct the state (ie. Collision and other constraints)
     * 3. Set the state queries, this is done in conjunction with collision correction
     */

    //TEST
    // Update based on input and state
    // players.update();

    // Correction and set state
    // players.terrain_collision(terrain);
    player_set.update_motion();
    player_set.update_collision();
    player_set.update_terrain_collision(&terrain);
    player_set.apply_bouyant_force(water.getWaterLevel());

    plant_system.update(terrain, water.getWaterLevel());
    entity_system.update();

}

void Scene::init_client(Client *client){
    // This is only for the client, assets are unused by server
    init_assets();
    sky.setSunDirection(0,1,0);
    water.setWaterLevel(4);
    terrain.generate();
    terrain.loadVAO(terrain_vao);
    entity_system.init();
    player_set.clear();
    plant_system.init(terrain, water.getWaterLevel());
}

void Scene::init_server(Server *server){
    //TODO load server data from file
    sky.setSunDirection(0,1,0);
    water.setWaterLevel(4);
    terrain.generate();
    entity_system.init();
    player_set.clear();
    plant_system.init(terrain, water.getWaterLevel());
}

void Scene::close_client(){
    Sky::close_assets();
    Water::close_assets();
}

void Scene::close_server(){
    //TODO save server data
}


void Scene::init_assets(){
    Sky::init_assets();
    Water::init_assets();
    Player::init_assets();
    player_set.init_armatures();

    object_shader.load("test");
    plant_shader.load("plant");
    anim_shader.load("test_anim");
    terrain_shader.load("terrain");
    terrain.loadVAO(terrain_vao);

    entity_system.init_entity_assets();
}

void Scene::close_assets(){
    Sky::close_assets();
    Water::close_assets();
    Player::close_assets();
    object_shader.free();
    anim_shader.free();
    terrain_shader.free();
    plant_shader.free();
    entity_system.close_entity_assets();
}
