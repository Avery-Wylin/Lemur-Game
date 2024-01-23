#include "CollisionShape.h"
#include <iostream>

/*
 * GJK adapted from https://github.com/kevinmoran/GJK/blob/master/GJK.h
 * under MIT license
 */

#define GJK_MAX_ITER 64
void update_simplex3(vec3 &a, vec3 &b, vec3 &c, vec3 &d, unsigned int &simplex_dimension, vec3 &search_dir);
bool update_simplex4(vec3 &a, vec3 &b, vec3 &c, vec3 &d, unsigned int &simplex_dimension, vec3 &search_dir);
void epa( vec3 a, vec3 b, vec3 c, vec3 d, CollisionShape &shape_a, CollisionShape &shape_b, vec3 resolve);

bool CollisionShape::gjk(CollisionShape &shape_a, CollisionShape &shape_b, vec3 resolve){

    // Declare simplex
    vec3
    a = GLM_VEC3_ZERO_INIT,
    b = GLM_VEC3_ZERO_INIT,
    c = GLM_VEC3_ZERO_INIT,
    d = GLM_VEC3_ZERO_INIT;

    // Initial search direction
    vec3 search_dir, inv_search_dir;
    glm_vec3_sub(shape_b.pos, shape_a.pos, search_dir);
    if(glm_vec3_eq(search_dir,0))
        search_dir[0] = 1;
    glm_vec3_negate_to(search_dir, inv_search_dir);

    // First simplex point, subtract the support vectors of both shapes in direction
    vec3 s1,s2;
    shape_a.getSupportVector(search_dir, s1);
    shape_b.getSupportVector(inv_search_dir, s2);
    glm_vec3_sub(s1, s2, c);

    // Set the search direction towards the origin
    glm_vec3_copy(c, inv_search_dir);
    glm_vec3_negate_to(inv_search_dir,search_dir);

    // Second simplex point
    shape_a.getSupportVector(search_dir, s1);
    shape_b.getSupportVector(inv_search_dir, s2);
    glm_vec3_sub(s1, s2, b);

    // Exit if the furthest point did not cross the origin (no collision)
    if(glm_vec3_dot(b, search_dir) < 0){
        return false;
    }

    // Set the search direction perpendicular to the line segment towards the origin
    glm_vec3_sub(c, b, s1);
    glm_vec3_negate_to(b, s2);
    glm_vec3_cross(s1, s2, search_dir);
    glm_vec3_cross(search_dir, s1, search_dir);

    // If the origin is on the line segment, any normal search vector is valid
    if( glm_vec3_eq_eps(search_dir,0)){
        // Try X axis first
        vec3 axis = {1,0,0};
        glm_vec3_cross(s1, axis, search_dir);

        // Try Z axis second
        if(glm_vec3_eq_eps(search_dir, 0)){
            axis[0] = 0;
            axis[2] = -1;
            glm_vec3_cross(s1, axis,search_dir);
        }
    }
    glm_vec3_negate_to(search_dir, inv_search_dir);

    // Set the simplex dimension
    unsigned int simplex_dimension = 2;

    for(unsigned int i = 0; i < GJK_MAX_ITER; ++i){
        glm_vec3_negate_to(search_dir, inv_search_dir);
        shape_a.getSupportVector(search_dir, s1);
        shape_b.getSupportVector(inv_search_dir, s2);
        glm_vec3_sub(s1, s2, a);

        // New point could not enclose the origin
        if(glm_vec3_dot(a, search_dir) < 0){
            return false;
        }

        ++simplex_dimension;

        // Update triangle simplex
        if(simplex_dimension == 3){
            update_simplex3(a,b,c,d, simplex_dimension, search_dir);
        }
        // Update tetrahedron simplex
        else if( update_simplex4(a,b,c,d, simplex_dimension, search_dir) ){
            // If the overlap information is requested, compute EPA
            if(resolve){
                epa(a,b,c,d,shape_a, shape_b, resolve);
            }

            return true;
        }
    }
    // Simplex did not enclose the origin
    return false;
}

void update_simplex3(vec3 &a, vec3 &b, vec3 &c, vec3 &d, unsigned int &simplex_dimension, vec3 &search_dir){
    // Winding order of triangle is counter-clockwise where a is the most recent point

    // Find normal
    vec3 normal, ab, ac, t, to_origin;
    glm_vec3_sub(b,a, ab );
    glm_vec3_sub(c,a, ac );
    glm_vec3_cross( ab, ac,normal);

    // Find direction to origin from a
    glm_vec3_negate_to(a, to_origin);

    // Find the closest feature to origin
    simplex_dimension = 2;

    // If closest to edge AB
    glm_vec3_cross( ab,normal, t );
    if(glm_vec3_dot( t, to_origin) > 0){
        glm_vec3_copy(a,c);
        glm_vec3_cross(ab,to_origin,search_dir);
        glm_vec3_cross(search_dir, ab, search_dir);
        return;
    }

    // If closest to edge AC
    glm_vec3_cross( normal, ac, t );
    if(glm_vec3_dot( t, to_origin) > 0){
        glm_vec3_copy(a,b);
        glm_vec3_cross(ac,to_origin,search_dir);
        glm_vec3_cross(search_dir, ac, search_dir);
        return;
    }

    simplex_dimension = 3;

    // If the new position is above the triangle
    if(glm_vec3_dot(normal, to_origin) > 0){
        // Copy the points down (triangle is now bcd) and set the search direction above
        glm_vec3_copy(c,d);
        glm_vec3_copy(b,c);
        glm_vec3_copy(a,b);
        glm_vec3_copy(normal,search_dir);
        return;
    }
    // If the new position is below the triangle

    // Reverse the winding order (swaps c and d)
    glm_vec3_copy(b,d);
    glm_vec3_copy(a,b);
    glm_vec3_negate_to(normal, search_dir);
    return;
}

bool update_simplex4(vec3 &a, vec3 &b, vec3 &c, vec3 &d, unsigned int &simplex_dimension, vec3 &search_dir){
    // a is the top pyramid, bcd is the counter-clockwise winding triangle base
    // The origin must be above bcd and below a

    vec3 abc, acd, adb, t1, t2, to_origin;

    // Normals of each face (excluding base)
    glm_vec3_sub(b,a,t1);
    glm_vec3_sub(c,a,t2);
    glm_vec3_cross(t1,t2, abc);
    glm_vec3_sub(d,a,t1);
    glm_vec3_cross(t2,t1, acd);
    glm_vec3_sub(b,a,t2);
    glm_vec3_cross(t1,t2, adb);

    // Vector to origin from new point
    glm_vec3_negate_to(a, to_origin);

    simplex_dimension = 3;

    // Use the first face with the origin in front of it as the new simplex
    if(glm_vec3_dot(abc, to_origin) > 0){
        glm_vec3_copy(c,d);
        glm_vec3_copy(b,c);
        glm_vec3_copy(a,b);
        glm_vec3_copy(abc, search_dir);
        return false;
    }
    if(glm_vec3_dot(acd, to_origin) > 0){
        glm_vec3_copy(a,b);
        glm_vec3_copy(acd, search_dir);
        return false;
    }
    if(glm_vec3_dot(adb, to_origin) > 0){
        glm_vec3_copy(d,c);
        glm_vec3_copy(b,d);
        glm_vec3_copy(a,b);
        glm_vec3_copy(adb, search_dir);
        return false;
    }

    // The origin is enclosed by the faces
    return true;
}

#define EPA_EPSILON 0.01
#define EPA_MAX_FACES 64
#define EPA_MAX_LOOSE_EDGES 32
#define EPA_MAX_ITER 32

void epa( vec3 a, vec3 b, vec3 c, vec3 d, CollisionShape &shape_a, CollisionShape &shape_b, vec3 resolve){

    vec3 faces[EPA_MAX_FACES][4];

    vec3 t1,t2;
    glm_vec3_sub(b,a,t1);
    glm_vec3_sub(c,a,t2);

    // Initialize from GJK simplex
    // abc
    glm_vec3_copy(a, faces[0][0]);
    glm_vec3_copy(b, faces[0][1]);
    glm_vec3_copy(c, faces[0][2]);
    glm_vec3_cross(t1, t2, faces[0][3]);
    glm_vec3_normalize(faces[0][3]);

    // acd
    glm_vec3_copy(a, faces[1][0]);
    glm_vec3_copy(c, faces[1][1]);
    glm_vec3_copy(d, faces[1][2]);
    glm_vec3_sub(d,a,t1);
    glm_vec3_cross(t2, t1, faces[1][3]);
    glm_vec3_normalize(faces[1][3]);

    // adb
    glm_vec3_copy(a, faces[2][0]);
    glm_vec3_copy(d, faces[2][1]);
    glm_vec3_copy(b, faces[2][2]);
    glm_vec3_sub(b,a,t2);
    glm_vec3_cross(t1, t2, faces[2][3]);
    glm_vec3_normalize(faces[2][3]);

    // bdc
    glm_vec3_copy(b, faces[3][0]);
    glm_vec3_copy(d, faces[3][1]);
    glm_vec3_copy(c, faces[3][2]);
    glm_vec3_sub(d,b,t1);
    glm_vec3_sub(c,b,t2);
    glm_vec3_cross(t1, t2, faces[3][3]);
    glm_vec3_normalize(faces[3][3]);

    unsigned int face_count = 4, closest_face;
    vec3 search_dir, inv_search_dir, p, t;
    vec3 loose_edges[EPA_MAX_LOOSE_EDGES][2];
    for(unsigned int iteration = 0; iteration < face_count; ++iteration ){

        // Find the closest face to the origin
        float min_distance = glm_vec3_dot(faces[0][0], faces[0][3]);
        float distance;
        closest_face = 0;
        for(unsigned int i = 1; i < face_count; ++i ){
            distance = glm_vec3_dot(faces[i][0], faces[i][3]);
            if(distance < min_distance){
                min_distance = distance;
                closest_face = i;
            }
        }

        // Look along the normal of the closest face for a support vector
        vec3 sa,sb;
        glm_vec3_copy(faces[closest_face][3], search_dir);
        glm_vec3_negate_to(search_dir, inv_search_dir);
        shape_a.getSupportVector(search_dir, sa);
        shape_b.getSupportVector(inv_search_dir, sb);
        glm_vec3_sub(sa,sb,p);

        // The new point is within error from the origin, resolve by dot of point with closest face
        if(glm_vec3_dot(p, search_dir) - min_distance < EPA_EPSILON ){
            glm_vec3_scale(faces[closest_face][3], glm_vec3_dot(p, search_dir), resolve);
            return;
        }


        // Find all triangles that face the new point
        unsigned int loose_edge_count = 0;
        for(unsigned int i = 0; i < face_count; ++i ){

            // Remove the trinagle if it faces new point
            glm_vec3_sub(p,faces[i][0],t);
            if(glm_vec3_dot(faces[i][3], t) > 0){

                // Insert edges into loose edge list, if already present, remove the edge
                for(unsigned int e = 0; e < 3; ++e){
                    vec3 ce[2];
                    glm_vec3_copy(faces[i][e], ce[0]);
                    glm_vec3_copy(faces[i][(e+1)%3], ce[1]);
                    bool edge_found = false;

                    for(unsigned int le = 0; le < loose_edge_count; ++le){
                        // If the points in the current edge match the loose edge
                        if( glm_vec3_eqv(loose_edges[le][1], ce[0]) && glm_vec3_eqv(loose_edges[le][0], ce[1])){
                            // Replace current edge with last edge
                            glm_vec3_copy(loose_edges[loose_edge_count-1][0],loose_edges[le][0]);
                            glm_vec3_copy(loose_edges[loose_edge_count-1][1],loose_edges[le][1]);
                            --loose_edge_count;
                            edge_found = true;
                            // Breaks the loop
                            le = loose_edge_count;
                        }
                    }

                    // Add the edge if it does not already exist
                    if(!edge_found){
                        // Discontinue if there are too many edges added
                        if(loose_edge_count >= EPA_MAX_LOOSE_EDGES)
                            break;
                        glm_vec3_copy(ce[0], loose_edges[loose_edge_count][0]);
                        glm_vec3_copy(ce[1], loose_edges[loose_edge_count][1]);
                        ++loose_edge_count;
                    }
                }

                // Remove the current face from the list
                glm_vec3_copy(faces[face_count-1][0],faces[i][0]);
                glm_vec3_copy(faces[face_count-1][1],faces[i][1]);
                glm_vec3_copy(faces[face_count-1][2],faces[i][2]);
                glm_vec3_copy(faces[face_count-1][3],faces[i][3]);
                --face_count;
                --i;
            }
            // Triangle does not face new point
        }

        // Rebuild the polytope with the new point
        vec3 t1,t2;
        for(unsigned int i=0; i < loose_edge_count; ++i){
            if(face_count >= EPA_MAX_FACES)
                break;
            glm_vec3_copy( loose_edges[i][0], faces[face_count][0]);
            glm_vec3_copy( loose_edges[i][1], faces[face_count][1]);
            glm_vec3_copy( p, faces[face_count][2]);
            glm_vec3_sub(loose_edges[i][0], loose_edges[i][1], t1);
            glm_vec3_sub(loose_edges[i][0], p, t2);
            glm_vec3_cross( t1,t2, faces[face_count][3]);
            glm_vec3_normalize(faces[face_count][3]);

            // Ensure normal is correct, reverse the winding order
            if(glm_vec3_dot(faces[face_count][0], faces[face_count][3])+ .000001 < 0){
                glm_vec3_copy(faces[face_count][0], t1);
                glm_vec3_copy(faces[face_count][1], faces[face_count][0]);
                glm_vec3_copy(t1, faces[face_count][1]);
                glm_vec3_inv(faces[face_count][3]);
            }
            ++face_count;
        }
    }
    // EPA did not converge to meet error limit, return closest point
    glm_vec3_scale(faces[closest_face][3], glm_vec3_dot(faces[closest_face][0], faces[closest_face][3]), resolve);

    return;
}

