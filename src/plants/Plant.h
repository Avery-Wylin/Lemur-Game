#ifndef PLANT_H
#define PLANT_H

#include "PlantSpecies.h"
#include "PlantMeshGen.h"

typedef uint16_t PlantID;
namespace Plant{
    extern Mesh mesh_leaves, mesh_branches, mesh_flowers;

};

class SpeciesList{
    std::vector<PlantSpecies> list;
    uint8_t
    count = 0,
    empty_start = 0;

public:
    uint8_t add(Terrain &terrain, float water_level);
    void remove(uint8_t id);
    void clear();
    PlantSpecies* at(uint8_t id);
    void update(Terrain &terrain, float water_level);
    void draw(View &view);
};

/*
 * A plant system contains all plant species and instances.
 * Plants are accessed using a plant-id which encode the species and the instance.
 * When a plant is removed it attempts to remove itself from players using it. (It is cheaper to scan a small number of players)
 */
class PlantSystem{
    SpeciesList species;
    uint32_t update_cycle = 0;

public:
    void init(Terrain &terrain, float water_level);
    void update(Terrain &terrain, float water_level);
    void draw(View &view);
    PlantID get_closest_plant(vec3 pos);
    PlantInstance* get_plant(uint32_t plant_id);
};

#endif // PLANT_H
