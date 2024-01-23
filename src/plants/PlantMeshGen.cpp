#include "PlantMeshGen.h"
#include "Mesh.h"
#include <random>


namespace PlantMeshes {
    Mesh leaves, branches, flowers;
    void init() {
        leaves.clear();
        leaves.append_PLY("leaf_strelitzia_1");
        leaves.append_PLY("leaf_strelitzia_2");
        leaves.append_PLY("leaf_bamboo_1");
        branches.clear();
        branches.append_PLY("branch");
        branches.append_PLY("branch_strelitzia_1");
        branches.append_PLY("branch_bamboo_1");
        flowers.clear();
        flowers.append_PLY("flower_strelitzia_1");
        flowers.append_PLY("flower_strelitzia_2");
        flowers.append_PLY("flower_bamboo_1");
    }
};


static inline bool chance( std::mt19937 &mt, float chance ) {
    float v = ( float )mt() / mt.max();
    return v < chance;
}

static inline float random_01( std::mt19937 &mt ) {
    float v = ( float )mt() / mt.max();
    return v;
}

static inline float random_n11( std::mt19937 &mt ) {
    float v = ( float )mt() / mt.max() - 0.5f;
    return v;
}

void PlantMeshParameters::generate_branch( Mesh &m , uint32_t seed) {
    // Create branch saves
    std::queue<Branch> active_branches;
    std::vector<Branch> finished_branches;

    // Noise generator
    std::mt19937 mt(seed);

    // Axis
    vec3
    x_axis = {1,0,0},
    y_axis = {0,1,0},
    ny_axis = {0,-1,0},
    z_axis = {0,0,1};

    // Temporary storage vars
    PlantMeshParameters::Branch b, nb;

    // Place the root branch (no randomness)
    b = Branch();
    b.b[1] = root_branch_size;
    b.level = 0;
    glm_vec3_rotate(b.b,branch_angles[0],z_axis);
    active_branches.push(b);

    // Place additional root branches (with randomness)
    for(uint8_t i = 1; i < root_count;++i){
        b = Branch();
        b.b[1] = root_branch_size;
        b.level = 0;
        b.a[0] = root_scatter * random_n11(mt);
        b.a[2] = root_scatter * random_n11(mt);
        glm_vec3_rotate(b.a,branch_angles[0],z_axis);
        glm_vec3_rotate(b.b,branch_angles[0]+ branch_angle_randomness * random_n11( mt ),z_axis);
        glm_vec3_add(b.a,b.b,b.b);
        active_branches.push(b);
    }


    // Generate branches for the branch stack
    while(!active_branches.empty()){
        b = active_branches.front();
        active_branches.pop();

        // Move on to the finished branches
        finished_branches.push_back(b);

        // Max level reached, do not attempt to generate any more branches
        if(b.level >= 4)
            continue;

        // Attempt to generate branch splits
        // const float angle_step = branch_angles[b.level] / (branch_splits[b.level]-1);
        // float angle = -branch_angles[b.level]/2.0;
        for(uint8_t i = 0; i < branch_splits[b.level]; ++i){

            // Chance the new branch fails
            if( !chance(mt, branch_chances[b.level]) ){
                continue;
            }

            // If the branch suceeded create a new one and place it on the active stack
            nb = Branch();

            // Create a vector representing the branch size
            vec3 v = GLM_VEC3_ZERO_INIT;
            v[1] = 1;

            vec3 g;
            versor q;
            glm_vec3_sub(b.b,b.a,g);
            glm_vec3_lerp(g,y_axis,branch_upturn_factor,g);
            glm_vec3_normalize(g);
            glm_quat_from_vecs(v, g,q);
            v[1] = branch_sizes[b.level] + branch_size_randomness* random_01(mt);


            // If Fan, rotate only on z
            if(branch_fanned){
                glm_vec3_rotate( v, (i+.5)*branch_angles[b.level]/(branch_splits[b.level]) - branch_angles[b.level]/2.0 + branch_angle_randomness * random_n11( mt ), z_axis );
                glm_vec3_rotate( v, branch_angle_randomness * random_n11( mt ), x_axis );
            }
            // If not fan rotate around y
            else{
                glm_vec3_rotate( v, branch_angles[b.level]/2 + branch_angle_randomness * random_n11( mt ), z_axis );
                glm_vec3_rotate( v, branch_angle_randomness * random_n11( mt ), x_axis );
                glm_vec3_rotate( v, 6.283 * i/branch_splits[b.level], y_axis );
            }
                glm_quat_rotatev(q,v,v);

            // Copy the end of the parent to the start of the new
            glm_vec3_copy(b.b, nb.a);

            // Add the vector offset from a to b
            glm_vec3_add(nb.a, v, nb.b);

            // Increase the level
            nb.level = b.level + 1;

            // Push on to the active queue
            active_branches.push(nb);
        }
    }

    // Iterate over all completed branches and add leaves while creating the mesh
    mat4 tr;
    versor q;
    for(Branch &b : finished_branches){

        // Get the rotation of the branch
        vec3 v;
        glm_vec3_sub( b.b, b.a, v );
        glm_vec3_normalize( v );
        glm_quat_from_vecs( y_axis, v, q );

        // Place it into the matrix
        glm_quat_mat4(q, tr);

        // Scale to match the size of the branch
        vec3 s;
        glm_vec3_broadcast(glm_vec3_distance(b.a,b.b),s);
        s[0] *= branch_thickness;
        s[2] *= branch_thickness;
        glm_scale(tr, s);

        // Transform the matrix to the start of the branch
        glm_vec3_copy(b.a, tr[3]);

        // Append the branch mesh
        m.append_mesh_transformed( PlantMeshes::branches, branch_variant, tr);

        mat4 ltr;
        vec3 p;
        versor lq;
        for(uint8_t i = 0; i < leaves_per_branch; ++i){
            if(b.level == 0 || !chance(mt,leaf_chances[b.level-1]))
                continue;
            if(leaf_on_branch)
                glm_vec3_lerp(b.b,b.a,random_01(mt),p);
            else
                glm_vec3_copy(b.b,p);
            glm_quat_for(ny_axis, v, lq);

            for(uint8_t j = 0; j < leaf_cluster_count; ++j){


                if(leaf_fanned){
                    glm_quat_mat4(lq,ltr);
                    glm_rotate( ltr, (j+.5)*leaf_cluster_angle/leaf_cluster_count - leaf_cluster_angle/2.0 + leaf_angle_randomness * random_n11( mt ), z_axis );
                    glm_rotate( ltr, leaf_angle_randomness * random_n11( mt ), x_axis );
                    glm_rotate( ltr, leaf_angle_randomness * random_n11( mt ), y_axis );
                }
                else{
                    glm_quat_mat4(lq,ltr);
                    glm_rotate( ltr, 6.283 * j/leaf_cluster_count, y_axis );
                    glm_rotate( ltr, leaf_angle_randomness * random_n11( mt ), z_axis );
                    glm_rotate( ltr, leaf_cluster_angle/2 + leaf_angle_randomness * random_n11( mt ), x_axis );
                }

                glm_vec3_copy(p,ltr[3]);

                if(chance(mt,leaf_is_flower_chance)){
                    glm_scale_uni(ltr, flower_size + leaf_size_randomness*random_n11(mt));
                    m.append_mesh_transformed( PlantMeshes::flowers, flower_variant, ltr);
                }
                else{
                    glm_scale_uni(ltr, leaf_size + leaf_size_randomness*random_n11(mt));
                    m.append_mesh_transformed( PlantMeshes::leaves, leaf_variant, ltr);
                }
            }
        }
    }
}

void ClimbingScaffold::Branch::snap_pos_on_branch(float location, vec3 pos){
    // Find the location on the branch
    vec3 p;
    glm_vec3_lerp(a, b, location, p);

    // Put the position in local space of the branch
    glm_vec3_sub(pos, a, pos);

    // Find the vector from the branch to the position
    vec3 to_pos;
    glm_vec3_sub(b,a, to_pos );
    glm_vec3_proj(pos, to_pos, to_pos );
    glm_vec3_sub(pos, to_pos, to_pos );

    // Adjust tb to match the radius
    glm_vec3_normalize( to_pos );
    glm_vec3_scale(to_pos, radius, to_pos);

    // Apply to the branch pos and save
    glm_vec3_add(p, to_pos, pos);

}

void ClimbingScaffold::clear(){
    branches.clear();
}

// TODO all values are currently hardcoded, use parameters
void ClimbingScaffold::generate_scaffold(){
    clear();

    float root_size = 12;

    // Place the root branch
    Branch root;
    root.b[1] = root_size;
    branches.push_back( root );

    std::mt19937 mt;

    // Add children to the root
    Branch b;

    for( uint8_t i = 0; i < 5; ++i ) {
        // Check parent for max children
        if(root.child_count >= MAX_CHILD_BRANCHES)
            break;

        // Create a new branch
        b = Branch();

        // Create a new branch
        b.location_parent = i*.5/5+.5;
        b.parent = 0;

        // Interpolate on the parent
        glm_vec3_lerp(root.a,root.b,b.location_parent, b.a);

        // Make a ranndom vector for the branch direction and project on to parent
        b.a[0] = cos(6.28*i/5);
        b.a[1] = 0;
        b.a[2] = sin(6.28*i/5);
        b.radius = .5*root.radius;
        glm_vec3_normalize(b.a);
        b.a[1] = 0;
        glm_vec3_scale(b.a, 2*random_01(mt)+1, b.b);
        root.snap_pos_on_branch(b.location_parent, b.a);
        glm_vec3_add(b.a, b.b, b.b);

        // Add to branches
        branches.push_back(b);

        // Add to parent
        root.child_ids[root.child_count] = branches.size()-1;
        root.child_locations[root.child_count] =  b.location_parent;
    }
}

void ClimbingScaffold::generate_mesh(Mesh &m, PlantMeshParameters &pmp){
    m.clear();

    versor q;
    vec3 x_axis = {1,0,0};
    vec3 v;
    mat4 tr;
    Mesh bm;

    // for(uint8_t i = 0; i < branches.size(); ++i){
    //     Branch &b = branches[i];
    //
    //     // Vector of branch
    //     glm_vec3_sub(b.b,b.a,v);
    //
    //     // Find rotation aligned to the branch
    //     glm_vec3_normalize(v);
    //     glm_quat_from_vecs(y_axis, v, q);
    //
    //     // Copy to tansform matrix
    //     glm_translate_make(tr,b.a);
    //     glm_quat_rotate(tr, q, tr);
    //     vec3 s = {b.radius, glm_vec3_distance(b.a,b.b), b.radius};
    //     glm_scale(tr, s);
    //
    //     pmp.generate_branch(bm);
    //     bm.merge_partitions();
    //
    //     // Add mesh
    //     if(i == 0)
    //         m.append_mesh_transformed(PlantMeshes::branches, 0, tr);
    //     else
    //         m.append_mesh_transformed(bm, 0, tr);
    // }

    // Rotate to +X using -Z rotation
    pmp.branch_angles[0] = -1.5;
    pmp.branch_upturn_factor = .6;

    for(uint8_t i = 0; i < branches.size(); ++i){
        Branch &b = branches[i];

        // Add mesh
        if(i == 0){
            glm_translate_make(tr,b.a);
            vec3 s = {b.radius, glm_vec3_distance(b.a,b.b), b.radius};
            glm_scale(tr, s);
            m.append_mesh_transformed(PlantMeshes::branches, 0, tr);
        }
        else{
            pmp.root_branch_size = glm_vec3_distance(b.a,b.b);
            pmp.branch_sizes[0] = pmp.root_branch_size*.7;
            pmp.branch_sizes[1] = pmp.branch_sizes[0]*.7;
            pmp.branch_sizes[2] = pmp.branch_sizes[1]*.7;
            pmp.branch_sizes[3] = pmp.branch_sizes[2]*.7;
            pmp.branch_thickness = b.radius/pmp.root_branch_size;

            // Find rotation aligned to the branch
            glm_vec3_sub(b.b,b.a,v);
            glm_vec3_normalize(v);
            glm_quat_from_vecs(x_axis, v, q);

            // Copy to tansform matrix
            glm_translate_make(tr,b.a);
            glm_quat_rotate(tr, q, tr);


            pmp.generate_branch(bm, i*51);
            bm.merge_partitions();
            m.append_mesh_transformed(bm, 0, tr);
        }
    }
}
