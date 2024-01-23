#include "EntitySystem.h"
#include "EntityRegistry.h"

void EntitySystem::add_entity_type( EntityType *e){
    if(type_count >= MAX_TYPES){
        printf("Maximum number of entity types reached.");
        return;
    }

    entity_types[type_count] = e;
    e->set_type_id(type_count);
    ++type_count;
}

void EntitySystem::init_types(){
    // Write all used types in order, changing order changes type id
    add_entity_type( new TypeBeachBall());
}

void EntitySystem::close_types(){
    // Remove all entity type pointers
    for(EntityTypeID i = 0; i < type_count; ++i){
        delete entity_types[i];
    }
    type_count = 0;
}

void EntitySystem::init_entity_assets(){

}

void EntitySystem::close_entity_assets(){

}


void EntitySystem::update(){

    physics.update();

}

void EntitySystem::draw(){
    physics.debug_draw();
}

void EntitySystem::init(){
    // Create each type
    init_types();

    // Initialize the physics system
    physics.init();
}
