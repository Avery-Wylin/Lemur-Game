#include "Plant.h"
#include "Shader.h"
#include <queue>


uint8_t SpeciesList::add(Terrain &terrain, float water_level){
    if(count >= PLANT_MAX_SPECIES)
        return PLANT_MAX_SPECIES;
    if(list.empty())
        list.push_back(PlantSpecies());

    PlantSpecies p;
    p.init(terrain,water_level);
    uint8_t new_id = empty_start;
    list[new_id] = p;
    ++count;
    for(uint8_t i = empty_start; i < list.size(); ++i){
        if(list[i].empty()){
            empty_start = i;
            return new_id;
        }
    }

    list.push_back(PlantSpecies());
    empty_start = list.size()-1;
    return new_id;
}

void SpeciesList::remove(uint8_t id){
    if(id >= list.size() || list[id].empty())
        return;
    list[id].clear();
    empty_start = empty_start>id?id:empty_start;
}

void SpeciesList::clear(){
    list.clear();
    empty_start = 0;
}

PlantSpecies* SpeciesList::at(uint8_t id){
    if(id >= list.size() || list[id].empty())
        return nullptr;
    return &list[id];
}

void SpeciesList::update(Terrain &terrain, float water_level){
    for(PlantSpecies &s : list){
        if(s.empty())
            continue;
        s.update(terrain,water_level);
    }
}

void SpeciesList::draw(View &view){
    for(PlantSpecies &s : list){
        s.draw(view);
    }
};

// Plant System

void PlantSystem::init(Terrain &terrain, float water_level){
    PlantMeshes::init();
    species.add(terrain, water_level);
}

void PlantSystem::update(Terrain &terrain, float water_level){
    if(update_cycle >= 64){
        species.update(terrain, water_level);
    }
    ++update_cycle;
}

void PlantSystem::draw(View &view){
    species.draw(view);
}

// TODO this is not efficient, just a test
PlantID PlantSystem::get_closest_plant(vec3 pos){
}

PlantInstance* PlantSystem::get_plant(uint32_t plant_id){
    if(plant_id == PLANT_NULL)
        return nullptr;

    return nullptr;
}

