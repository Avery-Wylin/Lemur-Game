#ifndef DBVH_H
#define DBVH_H

#include <cglm/cglm.h>
#include <cmath>
#include <vector>
#include <queue>
#include <algorithm>
#include "PhysicsTypes.h"
#include <View.h>

/*
 * An object ID is used to link the physics object to the DBVH.
 * Object IDs will start at 0 and range to the length of the object list.
 * Object IDs are only unique to each physics type.
 * Node IDs are the internal index of the DBVH.
 */

static constexpr uint32_t MAX_NODE_COUNT = UINT32_MAX - 1;


/*
 * Bounding box structure.
 * Can compute whether a box intersects another, volume, and the union box.
 */
class AABB {
        // float x1, y1, z1, x2, y2, z2;
        vec3 bounds[2];

    public:
        AABB();
        AABB( vec3 a,  vec3 b );

        void set( vec3 a,  vec3 b );
        bool intersects( AABB & ) ;
        float volume() ;
        AABB operator|( AABB & );
        void expand( float f );
        void expand( vec3 f );
        void translate( vec3 pos );
        void print();
        void debug_draw();
        void debug_draw( vec3 pos );
        bool in_frustum( vec4 *frustum_planes );
        bool in_frustum( vec4 *frustum_planes, vec3 pos );
        float dist_to_center(vec3 pos);
};

struct Node {
    AABB aabb;
    NodeID parent = null_node;
    NodeID child1 = null_node;
    NodeID child2 = null_node;
    ObjectID oid = null_object;
    float inheritCost = 0;

    Node() {
        parent = null_node;
        child1 = null_node;
        child2 = null_node;
        oid = null_object;
        inheritCost = 0;
    };
    Node( const AABB a, NodeID p, ObjectID oid ):
        aabb( a ), parent( p ), oid( oid ) {
        child1 = null_node;
        child2 = null_node;
        inheritCost = 0;
    };

    bool hasChildren() {
        return child1 != null_node && child2 != null_node;
    };

    bool hasChild() {
        return child1 != null_node || child2 != null_node;
    };

    bool hasParent() {
        return parent != null_node;
    };

    bool isLeaf() {
        return oid != null_object;
    };
};

struct NodeQueue {
        static constexpr unsigned int size = 100;
    private:
        NodeID data[size];
        unsigned int start = 0;
        unsigned int end = 0;

    public:
        inline void clear() {
            start = 0;
            end = 0;
        }

        inline void push( NodeID id ) {
            if( ( end + 1 ) % size == start ) {
                puts( "Node queue is full." );
                fflush( stdout );
                return;
            }

            data[end] = id;
            end = ( end + 1 ) % size;
        }

        inline void pop() {
            start = ( start + 1 ) % size;
        }

        inline bool empty() {
            return start == end;
        }

        inline NodeID front() {
            return data[start];
        }

        inline NodeID back() {
            return data[( size + end - 1 ) % size];
        }
};

/*
 * Dynamic Bounding Volume Hierarchy Tree.
 * NOTE This is not a thread-safe structure
 * Steps can be taken to make it so, but at a cost of serial performance
 */
class DBVH {
        std::vector<Node> nodes;
        uint32_t size;
        uint32_t leaf_count;
        NodeID empty_start_index = 0;
        NodeID root_index = 0;
        NodeID nextEmpty();
        void clear_node( NodeID );

    public:

        DBVH();

        // Inserts a new AABB and returns its index
        NodeID insert( AABB &aabb, ObjectID oid );

        // Replace a node's object id given its node id
        void set_object( NodeID id, ObjectID oid );

        // Removes a node given its index
        void remove( NodeID id );

        Node* at( NodeID id );

        // Get the node ID from an object ID
        NodeID get_nodeId( ObjectID );

        // Get intersecting leaf nodes
        void get_intersecting( AABB &aabb, ObjectID *return_values, unsigned int &return_count, unsigned int max_return_count );
        void get_intersecting( NodeID id, ObjectID *return_values, unsigned int &return_count, unsigned int max_return_count );

        // Debug
        void print();
        void debug_draw();

        inline NodeID get_root() {return root_index;};

};

#endif // DBVH_H
