#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include "../physics/PhysicsTypes.h"

/*
 * An Entity class contains a set of instances and the functions they use to interact.
 * Entities are not a singular instance in themselves, but rather a container and interface for what they represent.
 * Entities must separate themselves into 2 parts, the rendering infrastructure dependent on a GL context, and the computational infrastructure.
 * If an entity contains a physics object, it can query that object separately in the update step.
 */
class EntityType {
        EntityTypeID type_id = null_type;
    const char* name = "Null";

public:

    EntityType(){};
    virtual ~EntityType(){};

    /*
     * Assets are loaded before anything is run and closed on termination.
     */
    virtual void init_assets() = 0;
    virtual void close_assets() = 0;

    /*
     * The draw function draws all instances.
     * Unless multiple shaders are allowed,
     * it is best to assume the shader is a standard shader that has global uniforms pre-bound;
     */
    virtual void draw() = 0;

    /*
     * The basic update function.
     * This is called once per cycle.
     */
    virtual void update() = 0;

    void set_type_id( EntityTypeID tid){
        if(type_id != null_type){
            printf("Type id already set for %s.\n", name);
            return;
        }
        type_id = tid;
    }
};

#endif //ENTITY_H
