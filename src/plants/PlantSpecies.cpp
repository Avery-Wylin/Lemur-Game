#include "Plant.h"
#include "PlantSpecies.h"
#include "Shader.h"
#include <queue>
#include <random>


static inline float random_01( std::mt19937 &mt ) {
    float v = ( float )mt() / mt.max();
    return v;
}

static inline float random_n11( std::mt19937 &mt ) {
    float v = ( float )mt() / mt.max() - 0.5f;
    return v;
}


// Plant Species
void PlantSpecies::generate_mesh(){
    // Clear the mesh so it can be refilled
    mesh.clear();

    // Choose the generating function by type
    switch(type){

        // Bushes are like tiny trees
        case TYPE_BUSH:
        break;

        case TYPE_STRELITZIA:
        break;

        case TYPE_SUCCULENT:
        break;

        case TYPE_FLOWER:
        break;

        case TYPE_BAMBOO:
        break;

        case TYPE_GRASS:
        break;

        case TYPE_WATER:
        break;

        case TYPE_TREE_FRUIT:
        break;

        case TYPE_TREE:
        break;

        case TYPE_PALM:
        break;

    }

    PlantMeshParameters pmp;
    // bmp.generate_branch(mesh);

    ClimbingScaffold cs;
    cs.generate_scaffold();
    cs.generate_mesh(mesh, pmp);
    mesh.merge_partitions();
    bounding_box = mesh.get_bounding_box(0);

    // Load to VAO
    mesh.to_VAO(vao.get());
}

void PlantSpecies::init(Terrain &terrain, float water_level){
    vao.reset();
    vao = std::shared_ptr<VAO>(new VAO());
    generate_mesh();
    is_empty = false;

    // TEST just testing plants
    PlantInstance p;
    std::mt19937 mt;
    AABB bb;
    for(uint32_t i = 0; i < 100; ++i){
        p.pos[0] = 400.0f*random_n11(mt) + (TERRAIN_DIM*TERRAIN_SCALE*.5);
        p.pos[2] = 400.0f*random_n11(mt) + (TERRAIN_DIM*TERRAIN_SCALE*.5);
        terrain.pointProjection(p.pos);
        if(p.pos[1] < water_level){
            --i;
            continue;
        }
        p.y_rot =  6.28f*random_01(mt);
        bb = bounding_box;
        bb.translate(p.pos);
        dbvh.insert(bb, instances.size());
        instances.push_back(p);
    }
}

void PlantSpecies::draw(View &view){
    if(empty())
        return;
    vao->bind();
    mat4 transform;
    versor v = GLM_QUAT_IDENTITY_INIT;
    vec3 up = {0,1,0};
    vec4 *frustum =  view.get_frustum_planes(.7);
    int draw_count = 0, iterations = 0;

    // for(PlantInstance &p : instances){
    //     iterations++;
    //     if(glm_vec3_distance(view.pos,p.pos)>VIEW_FAR*.2)
    //         continue;
    //     if(!bounding_box.in_frustum(frustum, p.pos))
    //         continue;
    //     draw_count++;
    //     bounding_box.debug_draw(p.pos);
    //     DebugDraw::axis(v, p.pos);
    //     glm_rotate_make(transform, p.y_rot, up);
    //     glm_vec3_copy(p.pos, transform[3]);
    //     Shader::uniformMat4f(UNIFORM_TRANSFORM, transform);
    //     glDrawElements( GL_TRIANGLES, vao->getIndexCount(), GL_UNSIGNED_INT, 0 );
    // }


    static NodeQueue nq;
    nq.clear();
    nq.push(dbvh.get_root());
    Node *n;
    while( !nq.empty()){
        iterations++;
        n = dbvh.at(nq.front());
        nq.pop();
        if(n && n->aabb.in_frustum(frustum)){
            if(n->isLeaf()){
                PlantInstance &p = instances[n->oid];
                draw_count++;
                // n->aabb.debug_draw();
                // DebugDraw::axis(v, p.pos);
                glm_rotate_make(transform, p.y_rot, up);
                glm_vec3_copy(p.pos, transform[3]);
                Shader::uniformMat4f(UNIFORM_TRANSFORM, transform);
                glDrawElements( GL_TRIANGLES, vao->getIndexCount(), GL_UNSIGNED_INT, 0 );
            }
            else if(n->hasChildren()){
                nq.push(n->child1);
                nq.push(n->child2);
            }
        }
    }

    printf("dc:%d it:%d\n",draw_count,iterations);
    fflush(stdout);
}

void PlantSpecies::update(Terrain &terrain, float water_level){
    for(PlantInstance &p : instances){
        // TODO update plants here
    }
}

void PlantSpecies::clear() {
    is_empty = true;
    instances.clear();
    climbing_scaffold.clear();
    mesh.clear();
    vao->free();
}
