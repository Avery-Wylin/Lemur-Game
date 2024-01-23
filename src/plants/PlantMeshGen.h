#ifndef PLANTMESHGEN_H
#define PLANTMESHGEN_H

#include "PlantMeshGen.h"
#include <cglm/vec3.h>
#include <cglm/vec4.h>
#include <inttypes.h>
#include <Mesh.h>
#include <CollisionShape.h>

namespace PlantMeshes{
  extern Mesh leaves, branches, flowers;
  void init();
};

struct PlantMeshParameters {

    struct Branch{
        vec3
        a = GLM_VEC3_ZERO_INIT,
        b = GLM_VEC3_ZERO_INIT;
        uint8_t level = 0;
    };

    // Variety Parameters
    uint8_t
    branch_variant = 0,
    leaf_variant = 0,
    flower_variant = 0;

    // Leaf and Flower Parameters

    uint8_t leaf_cluster_count = 3;
    float leaf_cluster_angle = 3;
    float leaf_size = 1.5;
    float flower_size = .6;
    vec4 leaf_chances = {0, .4, .7, .9};
    float leaf_is_flower_chance = .1;
    float leaf_angle_randomness = 2;
    float leaf_size_randomness = 0.3;
    uint8_t leaves_per_branch = 2;
    bool leaf_on_branch = true;
    bool leaf_fanned = true;


    // Branching Parameters

    uint8_t root_count = 1;
    float root_scatter = 0;

    uint8_t branch_splits[4] = {2, 2, 2, 2};
    vec4 branch_chances = {1,.9,.9,.5};

    vec4 branch_angles = {0,1,1,1};
    float branch_angle_randomness = .5;
    bool branch_fanned = false;
    float branch_upturn_factor = .4;

    float root_branch_size = 1;
    vec4 branch_sizes = {1,.6,.5,.4};
    float branch_size_randomness = .2;
    float branch_thickness = .125;

    // Generate the branch mesh
    void generate_branch(Mesh& m, uint32_t seed = 5489);
};

struct ClimbingScaffold{
    static const uint8_t MAX_CHILD_BRANCHES = 8;
    struct Branch{
        vec3
        a = {0,0,0},
        b = {0,0,0};
        uint8_t parent = 0;
        uint8_t child_count = 0;
        uint8_t child_ids[MAX_CHILD_BRANCHES];
        float child_locations[MAX_CHILD_BRANCHES];
        float location_parent = 0;
        float radius = .25;

        void snap_pos_on_branch(float location, vec3 pos);
    };
    Capsule root_collider;
    std::vector<Branch> branches;

    void clear();
    void generate_scaffold();
    void generate_mesh(Mesh &m, PlantMeshParameters &pmp);
};


#endif // PLANTMESHGEN_H
