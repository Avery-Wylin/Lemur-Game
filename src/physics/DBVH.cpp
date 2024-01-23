#include "physics/DBVH.h"
#include <iostream>
#include "DebugDraw.h"


AABB::AABB() {
    glm_vec3_zero(bounds[0]);
    glm_vec3_zero(bounds[1]);
}

AABB::AABB( vec3 a, vec3 b ) {
    set( a, b );
}

void AABB::set( vec3 a, vec3 b ) {
    bounds[0][0] = glm_min( a[0], b[0] );
    bounds[0][1] = glm_min( a[1], b[1] );
    bounds[0][2] = glm_min( a[2], b[2] );
    bounds[1][0] = glm_max( a[0], b[0] );
    bounds[1][1] = glm_max( a[1], b[1] );
    bounds[1][2] = glm_max( a[2], b[2] );
}

bool AABB::intersects( AABB &a){
    return glm_aabb_aabb(bounds, a.bounds);
}

float AABB::volume() {
    return ( bounds[1][0] - bounds[0][0] ) * ( bounds[1][1] - bounds[0][1] ) * ( bounds[1][2] - bounds[0][2] );
}

AABB AABB::operator|( AABB &a ) {
    AABB r;
    glm_aabb_merge(bounds, a.bounds, r.bounds);
    return r;
}

void AABB::expand(float f){
    glm_vec3_subs(bounds[0],f,bounds[1]);
    glm_vec3_adds(bounds[1],f,bounds[0]);
}

void AABB::expand(vec3 f){
    glm_vec3_sub(bounds[0],f,bounds[1]);
    glm_vec3_add(bounds[1],f,bounds[0]);
}

void AABB::translate(vec3 pos){
    glm_vec3_add(bounds[0],pos,bounds[0]);
    glm_vec3_add(bounds[1],pos,bounds[1]);
}

void AABB::print() {
    printf( "{%.2f, %.2f, %.2f},{%.2f, %.2f, %.2f}", bounds[0][0], bounds[0][1], bounds[0][2], bounds[1][0], bounds[1][1], bounds[1][2] );
}

void AABB::debug_draw(){
    DebugDraw::box(bounds[0][0], bounds[0][1], bounds[0][2], bounds[1][0], bounds[1][1], bounds[1][2]);
}

void AABB::debug_draw(vec3 pos){
    DebugDraw::box(pos[0] + bounds[0][0], pos[1]+bounds[0][1], pos[2]+bounds[0][2], pos[0]+bounds[1][0], pos[1]+bounds[1][1], pos[2]+bounds[1][2]);
}

bool AABB::in_frustum(vec4* frustum_planes, vec3 pos){
    vec3 a[2];
    glm_vec3_add(bounds[0],pos,a[0]);
    glm_vec3_add(bounds[1],pos,a[1]);

    return glm_aabb_frustum(a,frustum_planes);
}

bool AABB::in_frustum(vec4* frustum_planes){
    return glm_aabb_frustum(bounds,frustum_planes);
}

float AABB::dist_to_center(vec3 pos){
    vec3 p;
    glm_aabb_center(bounds,p);
    return glm_vec3_distance(p,pos);
}


DBVH::DBVH() {
    nodes.push_back(Node());
    leaf_count = 0;
}

// Create a static node queue, it is cleared every use for insertion
static NodeQueue node_queue;

NodeID DBVH::insert( AABB &aabb, ObjectID oid ) {

    // Two new nodes will be added to the assumed node count
    if(nodes.size()+2 >= MAX_NODE_COUNT)
        return null_node;

    // If root is not a leaf and has no children, make it a leaf
    // printf("rn: %d\n",root_index);
    if( !nodes[root_index].isLeaf() && !nodes[root_index].hasChildren()) {
        nodes[root_index].oid = oid;
        nodes[root_index].aabb = aabb;
        leaf_count++;
        return root_index;
    }

    // Root is a leaf or has 2 children
    NodeID sibling_id = root_index;
    float best_cost = (nodes[root_index].aabb | aabb).volume();
    float direct_cost = 0, combined_cost;
    node_queue.clear();

    // First node is root
    node_queue.push( root_index );
    Node *node, *parent;

    while( !node_queue.empty() ) {
        // Select the node
        node = &nodes[node_queue.front()];
        parent = node->hasParent() ? &nodes[node->parent] : nullptr;

        // Set direct cost to union of current node and inserted
        direct_cost = ( node->aabb | aabb ).volume();

        // Set inherit cost for this node to the difference in union volume
        node->inheritCost =  direct_cost - node->aabb.volume();

        // Set the base combined cost to the direct cost
        combined_cost = direct_cost;

        // Append the cummulative inherit cost of the parents
        if( parent ) {
            combined_cost += parent ->inheritCost;
            node->inheritCost += parent->inheritCost;
        }

        // Node had a better cost
        if( combined_cost <= best_cost ) {
            best_cost = combined_cost;
            sibling_id = node_queue.front();

            // Push children if possibly better cost
            if( node->hasChildren() && node->inheritCost + aabb.volume() < best_cost ) {
                node_queue.push( node->child1 );
                node_queue.push( node->child2 );
            }
        }

        // Remove the selected candidate
        node_queue.pop();
    }

    // Find 2 empty nodes, one becomes the new parent, the other the sibling
    NodeID new_parent_id = nextEmpty();
    nodes[new_parent_id].parent = root_index;
    NodeID new_id = nextEmpty();
    nodes[new_id].parent = root_index;

    // References are created after the list may have expanded from nextEmpty(), otherwise they will be invalid
    Node &new_parent = nodes[new_parent_id], &sibling = nodes[sibling_id];
    node = &nodes[sibling_id];

    // Transfer new parent to be a child of best node's parent
    new_parent.parent = node->parent;

    // If the parent is null, then this node is the new root, otherwise update the parent
    if( new_parent.parent == null_node ){
        root_index = new_parent_id;
    }
    // If the new parent is not a root, set it as a child
    else{

        if(nodes[new_parent.parent].child1 == sibling_id )
            nodes[new_parent.parent].child1 = new_parent_id;
        else
            nodes[new_parent.parent].child2 = new_parent_id;
    }

    // Update the sibling node in place
    new_parent.child1 = sibling_id;
    sibling.parent = new_parent_id;

    // Add the new node as the second child
    new_parent.child2 = new_id;
    nodes[new_id] = Node(aabb, new_parent_id, oid );

    // Traverse up the tree from the child and refit each parent's aabb
    while( new_parent_id != null_node){
        nodes[new_parent_id].aabb = nodes[nodes[new_parent_id].child1].aabb | nodes[nodes[new_parent_id].child2].aabb;
        new_parent_id = nodes[new_parent_id].parent;
    }

    leaf_count++;
    return new_id;
}

void DBVH::set_object(NodeID id, ObjectID oid){
    if(id == null_node || !nodes[id].hasParent())
        return;
    nodes[id].oid = oid;
}

void DBVH::clear_node(NodeID id){
    // Clear the node and update the first empty
    nodes[id] = Node();
    empty_start_index = empty_start_index < id ? empty_start_index : id;
}

/*
 * Nodes are removed in such a fashion that leaf nodes are left untouched (retain the same index).
 * Intermediate nodes may be removed.
 */
void DBVH::remove( NodeID node_id ) {

    // Ensure the id is in bounds
    if(node_id == null_node || node_id >= nodes.size() || !nodes[node_id].isLeaf())
        return;

    if( node_id == root_index ) {
        // Set the root back to 0 and clear the whole list
        nodes.clear();
        leaf_count = 0;

        // Create new empty root
        root_index = 0;
        nodes.push_back( Node() );

        // Set the empty start index (finds empty nodes faster)
        empty_start_index = root_index;

        return;
    }
    else {
        NodeID parent_id = nodes[node_id].parent;
        Node &parent = nodes[parent_id];
        NodeID sibling_id = parent.child1 == node_id ? parent.child2 : parent.child1;
        Node &sibling = nodes[sibling_id];

        if( sibling.isLeaf() ) {
            // Parent is an unecessary intermediate node, remove the parent and transfer ownership upwards
            if( !parent.hasParent() ) {
                // Case where the parent is root, make sibling root
                root_index = sibling_id;
                // Remove sibling's parent
                sibling.parent = null_node;
            }
            else {
                // Parent has parent, remove intermediate parent

                // Transfer ownership to parent's parent, figure out child value
                if( nodes[parent.parent].child1 == parent_id ) {
                    nodes[parent.parent].child1 = sibling_id;
                }
                else {
                    nodes[parent.parent].child2 = sibling_id;
                }
                // Update sibling's parent
                sibling.parent = parent.parent;
            }
            // Clear the parent
            clear_node( parent_id );
        }

        // If the sibling is not a leaf, parent the sibling's children to the parent and remove the sibling
        else {
            // Transfer sibling's children to parent
            parent.child1 = sibling.child1;
            parent.child2 = sibling.child2;
            nodes[parent.child1].parent = sibling.parent;
            nodes[parent.child2].parent = sibling.parent;

            // Clear the sibling
            clear_node( sibling_id );
        }
        // Clear the node
        clear_node(node_id);
    }

    // Reduce the leaf count
    --leaf_count;

    return;
}

Node* DBVH::at(NodeID id){
    if(id >= nodes.size())
        return nullptr;
    return &nodes[id];
}

NodeID DBVH::get_nodeId(ObjectID oid){
    for(NodeID i = 0; i < nodes.size(); ++i){
        if(nodes[i].oid == oid)
            return i;
    }
}

void DBVH::print(){
    Node *n;
    std::queue<NodeID> q;
    q.push(root_index);
    while(!q.empty()){
        n = &nodes[q.front()];
        printf("ID:%d L:%d P:%d C1:%d C2:%d V:%f ",
               q.front(),
               n->oid,
               n->parent,
               n->child1,
               n->child2,
               n->aabb.volume());
        n->aabb.print();
        if(n->hasChildren()){
            q.push(n->child1);
            q.push(n->child2);
        }
        else if(n->child1 != null_node || n->child2 != null_node){
            if(n->child1 != null_node)
                q.push(n->child1);
            if(n->child2 != null_node)
                q.push(n->child2);
            printf("ERROR, only 1 child!");
        }
        q.pop();
        printf("\n");
    }

    printf("\nSize:%zu Leaf Count:%d\n",nodes.size(),leaf_count);
}

void DBVH::debug_draw(){
    for(NodeID i = 0; i < nodes.size(); ++i){
        if(nodes[i].isLeaf())
            nodes[i].aabb.debug_draw();
    }
}

NodeID DBVH::nextEmpty() {
    // starting from the empty_start_index, find the next empty node
    for( NodeID i = empty_start_index; i < nodes.size(); ++i ) {
        if( !nodes[i].hasParent() && i != root_index ) {
            empty_start_index = i;
            return i;
        }
    }
    // Expand if no empty nodes were found
    nodes.push_back( Node() );
    empty_start_index = nodes.size()-1;
    return empty_start_index;
}

void DBVH::get_intersecting( AABB &aabb, ObjectID *return_values, unsigned int &return_count, unsigned int max_return_count ) {
    node_queue.clear();
    node_queue.push(root_index);
    return_count = 0;

    // NOTE start can equal end if the queue overflows as well as when it is empty
    while( return_count < max_return_count && !node_queue.empty() ) {
        Node &candidate = nodes[node_queue.front()];

        if( candidate.aabb.intersects( aabb ) ) {
            // If the node is an intersecting leaf, add it to the return_values
            if( candidate.isLeaf() ) {
                return_values[return_count] = candidate.oid;
                ++return_count;
            }
            // If the node is not a leaf and has children, add its children to the queue
            else if( candidate.hasChildren() ) {
                node_queue.push( candidate.child1);
                node_queue.push( candidate.child2);
            }
        }
        // continue into the queue
        node_queue.pop();
    }
}

void DBVH::get_intersecting( NodeID id, ObjectID *return_values, unsigned int &return_count, unsigned int max_return_count ) {
    if(id == null_node)
        return;

    node_queue.clear();
    node_queue.push(root_index);
    return_count = 0;

    // NOTE start can equal end if the queue overflows as well as when it is empty
    while( return_count < max_return_count && !node_queue.empty() ) {
        Node &candidate = nodes[node_queue.front()];

        if( id != node_queue.front() && candidate.aabb.intersects( nodes[id].aabb ) ) {

            // If the node is an intersecting leaf, add it to the return_values
            if( candidate.isLeaf() ) {
                return_values[return_count] = candidate.oid;
                ++return_count;
            }
            // If the node is not a leaf and has children, add its children to the queue
            else if( candidate.hasChildren() ) {
                node_queue.push( candidate.child1);
                node_queue.push( candidate.child2);
            }
        }
        // continue into the queue
        node_queue.pop();
    }
}


