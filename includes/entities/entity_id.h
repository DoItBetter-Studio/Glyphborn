#ifndef ENTITY_ID_H
#define ENTITY_ID_H

#include <stdint.h>
#include <stdbool.h>

// An entity handle is an index into the pool + a generation counter.
// The generation lets stored handles (e.g. AIState.target) detect that
// the slot they point to has been recycled for a different entity.
typedef struct
{
    uint32_t index;
    uint32_t generation;
} EntityID;

static const EntityID ENTITY_NULL = { 0xFFFFFFFF, 0xFFFFFFFF };

static inline bool entity_id_equal(EntityID a, EntityID b)
{
    return a.index == b.index && a.generation == b.generation;
}

static inline bool entity_id_is_null(EntityID id)
{
    return id.index == ENTITY_NULL.index && id.generation == ENTITY_NULL.generation;
}

// Every entity is one of these. There is intentionally no per-type struct --
// type only changes spawn data and which systems care about an entity, not
// its shape. A ship and a villager are both Entity.
typedef enum
{
    ENTITY_TYPE_PLAYER,
    ENTITY_TYPE_NPC,
    ENTITY_TYPE_ANIMAL,
    ENTITY_TYPE_PROJECTILE,
    ENTITY_TYPE_ITEM_DROP,
    ENTITY_TYPE_SHIP,
    ENTITY_TYPE_COUNT
} EntityType;

#endif // ENTITY_ID_H