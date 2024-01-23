#ifndef PLANTSPECIES_H
#define PLANTSPECIES_H

#include "cglm/vec3.h"
#include "Mesh.h"
#include "Terrain.h"
#include <memory>
#include <unordered_map>
#include "PlantMeshGen.h"


/*
 * A plant instance is listed within a plant species.
 * Each plant instance does not represent the type but rather common plant values.
 */
struct PlantInstance{
    vec3 pos;
    float y_rot;
    bool is_empty = true;
};

enum PlantType : uint8_t {
    TYPE_BUSH,
    TYPE_STRELITZIA,
    TYPE_SUCCULENT,
    TYPE_FLOWER,
    TYPE_BAMBOO,
    TYPE_GRASS,
    TYPE_WATER,
    TYPE_TREE_FRUIT,
    TYPE_TREE,
    TYPE_PALM
};

/*
 * A plant species defines the model and statistics used for each plant.
 * Individual instances are also tracked in the species.
 */

class PlantSpecies {

    std::shared_ptr<VAO>  vao = std::shared_ptr<VAO>(nullptr);
    Mesh mesh;
    std::vector<PlantInstance> instances;
    PlantType type = TYPE_BUSH;
    PlantMeshParameters mesh_parameters;
    ClimbingScaffold climbing_scaffold;
    bool is_empty = true;
    DBVH dbvh;
    AABB bounding_box;

public:
    PlantSpecies(){};
    void generate_mesh();
    void init(Terrain &terrain, float water_level);
    void draw(View &view);
    void update(Terrain &terrain, float water_level);
    void clear();
    inline bool empty(){return is_empty;}
};

#endif // PLANTSPECIES_H
