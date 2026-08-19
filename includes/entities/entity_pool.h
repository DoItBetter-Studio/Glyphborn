#ifndef ENTITY_POOL_H
#define ENTITY_POOL_H

#include "entity_id.h"
#include "entity.h"
#include <stdint.h>
#include <stdbool.h>

#define MAX_ENTITIES 1024

typedef struct
{
	Entity entities[MAX_ENTITIES];
	uint16_t generation[MAX_ENTITIES];

	int count;          // number of alive entities
	uint32_t free_head;  // index of first slot to try for the next create
} EntityPool;

// Lifecycle
void entity_pool_init(EntityPool* pool);
void entity_pool_shutdown(EntityPool* pool);

// Create a new entity. Index 0 is permanently reserved (ENTITY_NULL), so
// the pool holds MAX_ENTITIES - 1 usable entities.
EntityID entity_create(EntityPool* pool, EntityType type, const char* name);

// Marks the slot dead, bumps its generation (invalidating any stored
// EntityID that pointed at it -- e.g. an AIState.target), and zeroes its data.
void entity_destroy(EntityPool* pool, EntityID id);

bool entity_alive(const EntityPool* pool, EntityID id);

// Returns NULL if `id` is stale or out of range. Always check entity_alive()
// (or check for NULL here) before dereferencing a stored EntityID -- the
// generation check is what catches use-after-free style bugs from a
// recycled slot.
Entity* entity_get(EntityPool* pool, EntityID id);

// Linear scan for the entity occupying tile (x, y, z). Returns ENTITY_NULL
// if the tile is empty. Fine at this scale (<=1023 entities); revisit with
// a per-cell spatial index only if profiling says so.
EntityID entity_at_tile(EntityPool* pool, uint16_t x, uint16_t y, uint16_t z);

// Iterate every alive entity. fn may call entity_destroy on the entity it
// was given (its slot won't be revisited this pass), but must not destroy
// other entities mid-iteration -- queue those for after the loop.
typedef void (*EntityIterFn)(EntityPool* pool, EntityID id, void* user_data);
void entity_pool_each(EntityPool* pool, EntityIterFn fn, void* user_data);

#endif // !ENTITY_POOL_H