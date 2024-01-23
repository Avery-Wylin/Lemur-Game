#ifndef ENTITYSYSTEM_H
#define ENTITYSYSTEM_H

#include <vector>
#include "./EntityType.h"
#include "physics/PhysicsSystem.h"

static constexpr EntityTypeID MAX_TYPES = 254;
/*
 * A container for all entity types.
 */
class EntitySystem {
        EntityTypeID type_count = 0;
        EntityType *entity_types[MAX_TYPES];
        PhysicsSystem physics;

    // Initialize all entity types, an instance of each entity type is created in a vector and the type ids are hard-coded
    void init_types();
    void close_types();
    void add_entity_type( EntityType *e);

public:
    void update();
    void draw();
    void init();
    void close();
    void init_entity_assets();
    void close_entity_assets();
};

#endif // ENTITYSYSTEM_H
