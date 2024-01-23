#ifndef COLLISIONSHAPE_H
#define COLLISIONSHAPE_H

#include "../graphics/DebugDraw.h"
#include "DBVH.h"

struct CollisionShape {
    vec3 pos = GLM_VEC3_ZERO_INIT;
    versor rot = GLM_QUAT_IDENTITY_INIT;
    versor inv_rot = GLM_QUAT_IDENTITY_INIT;
    AABB aabb;
    virtual void updateAABB(){};
    virtual ~CollisionShape(){};
    virtual void getSupportVector( const vec3 direction, vec3 &dest ){
        glm_vec3_zero(dest);
    };
    virtual void debugDraw() {
        DebugDraw::axis( rot, pos );
    };


    static bool gjk( CollisionShape &shape_a, CollisionShape &shape_b, vec3 resolve = nullptr);
};

struct Sphere : CollisionShape {
    float radius = 1;
    void updateAABB() override{
        vec3 a, b, r = {radius,radius,radius};
        // Add and subtract the radius from the center position
        glm_vec3_add(pos, r, a);
        glm_vec3_sub(pos, r, b);
        aabb.set(a,b);
    }

    void getSupportVector( const vec3 direction, vec3 &dest ) override {
        vec3 dir = {direction[0],direction[1],direction[2]};

        glm_vec3_scale( dir, radius / glm_vec3_norm( dir ), dir );

        glm_vec3_add( dir, pos, dest );
    }
    void debugDraw() override {
        DebugDraw::polygon( rot, pos, radius, 16, 0 );
        DebugDraw::polygon( rot, pos, radius, 16, 1 );
        DebugDraw::polygon( rot, pos, radius, 16, 2 );
    };
};

struct Box : CollisionShape {
    vec3 half_extents = GLM_VEC3_ONE_INIT;

    void updateAABB() override{
        // Get half_extents
        vec3 a, b, ex = {half_extents[0], 0, 0}, ey = {0, half_extents[1], 0}, ez = {0, 0, half_extents[2]};

        // Rotate the extents
        glm_quat_rotatev(rot, ex, ex);
        glm_quat_rotatev(rot, ey, ey);
        glm_quat_rotatev(rot, ez, ez);

        // Combine the absolute value of each vector to get the furthest vector
        a[0] = abs(ex[0]) + abs(ey[0]) + abs(ez[0]);
        a[1] = abs(ex[1]) + abs(ey[1]) + abs(ez[1]);
        a[2] = abs(ex[2]) + abs(ey[2]) + abs(ez[2]);

        // Inverse to b, the other end of the box
        glm_vec3_inv_to(a,b);

        // translate
        glm_vec3_add(pos, a, a);
        glm_vec3_add(pos, b, b);

        aabb.set(a,b);
    }

    void getSupportVector( const vec3 direction, vec3 &dest ) override {
        vec3 dir = {direction[0],direction[1],direction[2]};
        glm_quat_rotatev(inv_rot, dir, dir);

        dest[0] =  dir[0] > 0 ? half_extents[0] : -half_extents[0];
        dest[1] =  dir[1] > 0 ? half_extents[1] : -half_extents[1];
        dest[2] =  dir[2] > 0 ? half_extents[2] : -half_extents[2];

        glm_quat_rotatev(rot, dest, dest);
        glm_vec3_add( dest, pos, dest );
    }
    void debugDraw() override {
        DebugDraw::box( rot, pos, -half_extents[0], -half_extents[1], -half_extents[2], half_extents[0], half_extents[1], half_extents[2] );
    };
};

struct Cylinder : CollisionShape {
    float radius = 1, y_extension = 1;

    void getSupportVector( const vec3 direction, vec3 &dest ) override {
        vec3 dir = {direction[0], direction[1], direction[2]};
        glm_quat_rotatev(inv_rot, dir, dir);

        vec3 dir_xz = {dir[0], 0, dir[2]};
        glm_vec3_normalize_to( dir_xz, dest );
        glm_vec3_scale( dest, radius, dest );
        dest[1] = dir[1] > 0 ? y_extension : -y_extension;

        glm_quat_rotatev(rot, dest, dest);
        glm_vec3_add( dest, pos, dest );
    }
     void debugDraw() override {
        vec3 p1 = {0,y_extension,0}, p2;
        glm_quat_rotatev(rot,p1,p1);
        glm_vec3_sub(pos,p1,p2);
        glm_vec3_add(pos,p1,p1);
        DebugDraw::polygon(rot, p1, radius, 16, 0);
        DebugDraw::polygon(rot, p2, radius, 16, 0);
        DebugDraw::line(p1[0],p1[1],p1[2], p2[0],p2[1],p2[2]);
    };

    void updateAABB() override{
        // Get half_extents, similar to a box, but in this case use radius for x and z
        vec3 a, b, ex = {radius, 0, 0}, ey = {0, y_extension, 0}, ez = {0, 0, radius};

        // Rotate the extents
        glm_quat_rotatev(rot, ex, ex);
        glm_quat_rotatev(rot, ey, ey);
        glm_quat_rotatev(rot, ez, ez);

        // Combine the absolute value of each vector to get the furthest vector
        a[0] = abs(ex[0]) + abs(ey[0]) + abs(ez[0]);
        a[1] = abs(ex[1]) + abs(ey[1]) + abs(ez[1]);
        a[2] = abs(ex[2]) + abs(ey[2]) + abs(ez[2]);

        // Inverse to b, the other end of the box
        glm_vec3_inv_to(a,b);

        // translate
        glm_vec3_add(pos, a, a);
        glm_vec3_add(pos, b, b);

        aabb.set(a,b);
    }

};

struct Capsule : CollisionShape {
    float radius = 1, y_extension = 1;

    void getSupportVector( const vec3 direction, vec3 &dest ) override {
        vec3 dir = {direction[0],direction[1],direction[2]};
        glm_quat_rotatev(inv_rot, dir, dir);

        glm_vec3_normalize_to( dir, dest );
        glm_vec3_scale( dest, radius, dest );

        dest[1] += dest[1] > 0 ? y_extension : -y_extension;
        glm_quat_rotatev(rot, dest, dest);
        glm_vec3_add( dest, pos, dest );
    }
    void debugDraw() override {
        vec3 p1 = {0,y_extension,0}, p2;
        glm_quat_rotatev(rot,p1,p1);
        glm_vec3_sub(pos,p1,p2);
        glm_vec3_add(pos,p1,p1);
        DebugDraw::polygon(rot, p1, radius, 16, 0);
        DebugDraw::polygon(rot, p1, radius, 16, 1);
        DebugDraw::polygon(rot, p2, radius, 16, 0);
        DebugDraw::polygon(rot, p2, radius, 16, 1);
        DebugDraw::line(p1[0],p1[1],p1[2], p2[0],p2[1],p2[2]);
    };
    void updateAABB() override{

        // Get the y extent and add the radius to it
        vec3 a = {0, y_extension+radius, 0}, b;

        // Rotate the y extent
        glm_quat_rotatev(rot, a, a);

        // Inverse to b, the other end of the box
        glm_vec3_inv_to(a,b);

        // translate
        glm_vec3_add(pos, a, a);
        glm_vec3_add(pos, b, b);

        aabb.set(a,b);
    }
};

struct Triangle : CollisionShape {
    vec3 points[3];

    void getSupportVector( const vec3 direction, vec3 &dest ) override {
        vec3 dir = {direction[0],direction[1],direction[2]};
        vec3 dot_prods;
        dot_prods[0] =  glm_vec3_dot( points[0], dir );
        dot_prods[1] =  glm_vec3_dot( points[1], dir );
        dot_prods[2] =  glm_vec3_dot( points[2], dir );


        if( dot_prods[1] > dot_prods[0] ) {
            if( dot_prods[2] > dot_prods[1] )
                glm_vec3_copy( points[2], dest );
            else
                glm_vec3_copy( points[1], dest );
        }
        else if( dot_prods[2] > dot_prods[0] )
            glm_vec3_copy( points[2], dest );
        else
            glm_vec3_copy( points[0], dest );

        vec3 n, ab, ac;
        glm_vec3_sub( points[1], points[0], ab );
        glm_vec3_sub( points[2], points[0], ac );
        glm_vec3_cross( ab, ac, n );

        // Depth behind triangle (prism like)
        if( glm_vec3_dot( dir, n ) < 0 )
            glm_vec3_sub( dest, n, dest );

        //translate
        glm_vec3_add( dest, pos, dest );
    }

     void updateAABB() override{

        vec3 a, b;

        // Find the min and max of all points
        a[0] = fmax(fmax(points[0][0], points[1][0]),points[2][0]);
        a[1] = fmax(fmax(points[0][1], points[1][1]),points[2][1]);
        a[2] = fmax(fmax(points[0][2], points[1][2]),points[2][2]);

        b[0] = fmin(fmin(points[0][0], points[1][0]),points[2][0]);
        b[1] = fmin(fmin(points[0][1], points[1][1]),points[2][1]);
        b[2] = fmin(fmin(points[0][2], points[1][2]),points[2][2]);

        // translate
        glm_vec3_add(pos, a, a);
        glm_vec3_add(pos, b, b);

        aabb.set(a,b);
    }
};



#endif // COLLISIONSHAPE_H

