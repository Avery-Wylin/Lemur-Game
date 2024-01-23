#ifndef PHYSICSTYPES_H
#define PHYSICSTYPES_H

#include <inttypes.h>

typedef uint32_t EntityID;  // The unique id given to each entity, first byte is type, reamining is the index
typedef uint8_t EntityTypeID; // The first bytes of an entity id
typedef uint32_t ObjectID;  // The physics object index, the user must know what type of physics object
typedef uint32_t NodeID;

static EntityID null_entity = UINT32_MAX;
static EntityTypeID null_type = UINT8_MAX;
static ObjectID null_object = UINT32_MAX;
static NodeID null_node = UINT32_MAX;

#endif //PHYSICSTYPES_H
