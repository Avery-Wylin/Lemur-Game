#include "PhysicsSystem.h"

void PhysicsSystem::init(){

}

void PhysicsSystem::update(){
    // Apply motion/transforms for dynamic dynamic_objects
}

void PhysicsSystem::debug_draw(){
    dynamic_dbvh.debug_draw();
    static_dbvh.debug_draw();
}

ObjectID PhysicsSystem::get_empty_dynamic() {
    for(ObjectID i = empty_dynamic_start; i < dynamic_objects.size(); ++i){
        if(dynamic_objects[i].empty())
            return i;
    }
    dynamic_objects.push_back(DynamicObject());
    return dynamic_objects.size()-1;
}

ObjectID PhysicsSystem::get_empty_static() {
    for(ObjectID i = empty_static_start; i < static_objects.size(); ++i){
        if(static_objects[i].empty())
            return i;
    }
    static_objects.push_back(StaticObject());
    return static_objects.size()-1;
}


ObjectID PhysicsSystem::create_object(const DynamicObject &d){
    //  Return if d does not have a collision shape
    if(dynamic_objects.size() >= MAX_DYNAMIC_OBJECTS || d.shape == nullptr)
        return null_object;

    // Get the next empty dynamic object, return if none found
    ObjectID oid = get_empty_dynamic();
    if(oid == null_object)
        return null_object;


    // Create a new DBVH node, if not possible it will return null_node
    NodeID nid = dynamic_dbvh.insert(d.shape->aabb, oid);
    if(nid == null_node)
        return null_object;

    // Copy the values of d into the new object
    dynamic_objects[oid] = d;

    // Set the node for the dynamic object and return the new object id (no longer empty)
    dynamic_objects[oid].dbvh_node = nid;
    return oid;
}

ObjectID PhysicsSystem::create_object(const StaticObject &s){
    //  Return if s does not have a collision shape
    if(static_objects.size() >= MAX_STATIC_OBJECTS || s.shape == nullptr)
        return null_object;

    // Get the next empty dynamic object, return if none found
    ObjectID oid = get_empty_static();
    if(oid == null_object)
        return null_object;

    // Create a new DBVH node, if not possible it will return null_node
    NodeID nid = static_dbvh.insert(s.shape->aabb, oid);
    if(nid == null_node)
        return null_object;

    // Copy the values of s into the new object
    static_objects[oid] = s;

    // Set the node for the dynamic object and return the new object id (no longer empty)
    static_objects[oid].dbvh_node = nid;
    return oid;
}

void PhysicsSystem::remove_dynamic_object(ObjectID oid){
    // Return if out of bounds
    if(oid == null_object || oid >= dynamic_objects.size())
        return;

    // Clear the object
    dynamic_objects[oid].clear();

    // Update the empty start search location
    if(empty_dynamic_start > oid)
        empty_dynamic_start = oid;
}

void PhysicsSystem::remove_static_object(ObjectID oid){
// Return if out of bounds
    if(oid == null_object || oid >= static_objects.size())
        return;

    // Clear the object
    static_objects[oid].clear();

    // Update the empty start search location
    if(empty_static_start > oid)
        empty_static_start = oid;
}

