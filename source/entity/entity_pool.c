#include "entities/entity_pool.h"
#include <string.h>
#include <assert.h>

void entity_pool_init(EntityPool* pool)
{
	memset(pool, 0, sizeof(EntityPool));

	// Slot 0 is permanently reserved as ENTITY_NULL and never marked alive.
	pool->free_head = 1;
	pool->count = 0;
}

void entity_pool_shutdown(EntityPool* pool)
{
	memset(pool, 0, sizeof(EntityPool));
}

EntityID entity_create(EntityPool* pool, EntityType type, const char* name)
{
	assert(pool->count < MAX_ENTITIES - 1 && "Entity pool exhausted");

	uint32_t idx = pool->free_head;

	// Advance free_head past whatever we just took, skipping any slots
	// that are still alive (shouldn't normally happen for idx+1, but a
	// destroy() earlier than free_head can leave gaps).
	pool->free_head = idx + 1;
	while (pool->free_head < MAX_ENTITIES && pool->entities[pool->free_head].alive)
		pool->free_head++;

	Entity* e = &pool->entities[idx];
	memset(e, 0, sizeof(Entity));
	e->alive = true;
	e->type = type;
	e->name = name;

	pool->count++;

	EntityID id = { idx, pool->generation[idx] };
	return id;
}

void entity_destroy(EntityPool* pool, EntityID id)
{
	if (!entity_alive(pool, id))
		return;

	uint32_t idx = id.index;

	memset(&pool->entities[idx], 0, sizeof(Entity));
	pool->generation[idx]++; // invalidate any stored EntityID for this slot

	if (idx < pool->free_head)
		pool->free_head = idx;

	pool->count--;
}

bool entity_alive(const EntityPool* pool, EntityID id)
{
	if (id.index == 0 || id.index >= MAX_ENTITIES)
		return false;

	return pool->entities[id.index].alive && pool->generation[id.index] == id.generation;
}

Entity* entity_get(EntityPool* pool, EntityID id)
{
	if (!entity_alive(pool, id))
		return NULL;

	return &pool->entities[id.index];
}

EntityID entity_at_tile(EntityPool* pool, uint16_t x, uint16_t y, uint16_t z)
{
	for (uint32_t i = 1; i < MAX_ENTITIES; i++)
	{
		Entity* e = &pool->entities[i];
		if (!e->alive)
			continue;

		if (e->transform.x == x && e->transform.y == y && e->transform.z == z)
		{
			EntityID id = { i, pool->generation[i] };
			return id;
		}
	}

	return ENTITY_NULL;
}

void entity_pool_each(EntityPool* pool, EntityIterFn fn, void* user_data)
{
	for (uint32_t i = 1; i < MAX_ENTITIES; i++)
	{
		if (!pool->entities[i].alive)
			continue;

		EntityID id = { i, pool->generation[i] };
		fn(pool, id, user_data);
	}
}