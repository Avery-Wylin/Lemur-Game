#ifndef PHYSICSSYSTEM_H
#define PHYSICSSYSTEM_H

#include "PhysicsTypes.h"
#include "CollisionShape.h"
#include "DBVH.h"

using std::vector;
static constexpr uint8_t MAX_CONTACTS = 4;
static constexpr uint32_t MAX_DYNAMIC_OBJECTS = 1000, MAX_STATIC_OBJECTS = 4000;

/*
 * Foundation class for other physics object classes.
 */
class PhysicalObject{
public:
    NodeID dbvh_node = null_node;
    CollisionShape *shape = nullptr;
    EntityID owner = null_entity;

    inline bool empty(){
        return dbvh_node == null_node || shape == nullptr;
    };

    inline void clear(){
        dbvh_node = null_node;
        shape = nullptr;
    }
};

/*
 * An object that provides a solid collision, but does not move.
 */
class StaticObject : public PhysicalObject{

};

/*
 * An object that can move and be pushed.
 */
class DynamicObject : public PhysicalObject{
public:
    ObjectID contacts[MAX_CONTACTS];
};

/*
 * Steps for physics updates (each step is a separate loop):
 * 1. Update the Object using logic/input and apply any corrections
 *    This step is done externally by an owning entity.
 *    Other external steps may include entity garbage collection and removal which should delete physics objects before they are run
 *    Entity data compaction is bidirectional, the entities change indices and then update the indicies of the respective physics object.
 *    DBVH data does not get compacted
 * 2. Object applies any motion or transformations, this is only for Dynamic Objects, if none/negligible, the object is marked as asleep
 * 3. Object calculates collisions with other object and calculates state (self correction and contacts), but do not apply them
 * 4. Apply any physics corrections
 * 5. Apply created/destroyed contact logic
 * 6. Render
 *
 *
 * For the collision calculation, assume motion has already been applied:
 * 1. For each awake dynamic object, run it through the dynamic object DBVH and create contact candidates
 *    Since the order is calculated as ascending object IDS, if the contact candidate is of a lesser id than the current, it can be ignored
 *    If the object ID is greater, then the collision is calculated and the contact info is shared with the other object
 *    If a contact is new, it can can tell the entity system which entities have created a contact and call their respective functions
 *    If a contact is removed, the same can be applied
 *    Any dynamic object involved in a contact will be marked as awake
 *
 * 2. Non-Dynamic objects can not initiate collisions, thus the same step is to be done for any contactable object type
 *    Dynamic objects will be run through the non-dynamic object BVH for contact candidates
 *    Object IDS can not be used to shortcut, so all collisions are considered
 *    If the non-dynamic object type is contactable, it will also receive a copy of the contact info from the dynamic object
 *    The entity system will also be notified of new or removed contacts that have been determined, these may be queued
 *    If the non-dynamic object is only a sensor (no collision), then the EPA function will not be called for the dynamic object
 *
 * 3. Physics corrections are called on dynamic objects, this uses the results of EPA in the contact info
 *
 * 4. Contacts that are added/removed are stored in a list and notify the entity system with the 2 entity IDs affected, the add/remove status, and the 2 local contact info ids
 *    The objects can safely change everything but their transformations
*/

class PhysicsSystem {
    DBVH dynamic_dbvh;
    DBVH static_dbvh;
    vector<DynamicObject> dynamic_objects;
    vector<StaticObject> static_objects;

    ObjectID
        empty_dynamic_start = 0,
        empty_static_start = 0;

    ObjectID get_empty_dynamic();
    ObjectID get_empty_static();
    void clear_dynamic();
    void clear_static();

public:
    void init();
    void update();
    void debug_draw();

    // Create an object using a template object,
    ObjectID create_object(const DynamicObject &d);
    ObjectID create_object(const StaticObject &s);

    // Remove an object by its id
    void remove_dynamic_object(ObjectID oid);
    void remove_static_object(ObjectID oid);

    // Get an object
    inline StaticObject* get_static_object(ObjectID oid){
        if(oid == null_node)
            return nullptr;
        return &static_objects[oid];
    }
    inline DynamicObject* get_dynamic_object(ObjectID oid){
        if(oid == null_node)
            return nullptr;
        return &dynamic_objects[oid];
    }
};

#endif // PHYSICSSYSTEM_H
