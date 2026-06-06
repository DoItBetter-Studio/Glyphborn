#include "world/world_tileset_cache.h"
#include <stdlib.h>

typedef struct CachedTileset
{
    uint16_t    id;
    Tileset*    tileset;
    uint32_t    ref_count;

    struct CachedTileset* next;
} CachedTileset;

static CachedTileset* g_regional_cache = NULL;
static CachedTileset* g_local_cache    = NULL;
static CachedTileset* g_interior_cache = NULL;

static Tileset* cache_acquire(CachedTileset** cache, uint16_t id, Tileset* (*loader)(uint16_t))
{
    CachedTileset* entry = *cache;

    while (entry)
    {
        if (entry-> id == id)
        {
            entry->ref_count++;
            return entry->tileset;
        }

        entry = entry->next;
    }

    Tileset* ts = loader(id);

    if (!ts) return NULL;

    CachedTileset* new_entry = calloc(1, sizeof(CachedTileset));

    new_entry->id        = id;
    new_entry->tileset   = ts;
    new_entry->ref_count = 1;

    new_entry->next = *cache;
    *cache = new_entry;

    return ts;
}

static void cache_release(CachedTileset** cache, uint16_t id, void (*freer)(Tileset*))
{
    CachedTileset** pp = cache;

    while (*pp)
    {
        CachedTileset* entry = *pp;

        if (entry->id == id)
        {
            entry->ref_count--;

            if (entry->ref_count == 0)
            {
                freer(entry->tileset);
                *pp = entry->next;
                free(entry);
            }

            return;
        }

        pp = &(*pp)->next;
    }
}

Tileset* tileset_acquire_regional(uint16_t id)
{
    return cache_acquire(&g_regional_cache, id, tileset_load_regional);
}

void tileset_release_regional(uint16_t id)
{
    cache_release(&g_regional_cache, id, tileset_free);
}

Tileset* tileset_acquire_local(uint16_t id)
{
    return cache_acquire(&g_local_cache, id, tileset_load_local);
}

void tileset_release_local(uint16_t id)
{
    cache_release(&g_local_cache, id, tileset_free);
}

Tileset* tileset_acquire_interior(uint16_t id)
{
    return cache_acquire(&g_interior_cache, id, tileset_load_interior);
}

void tileset_release_interior(uint16_t id)
{
    cache_release(&g_interior_cache, id, tileset_free);
}

void tileset_cache_shutdown(void)
{
    CachedTileset* caches[] = {
        g_regional_cache,
        g_local_cache,
        g_interior_cache
    };

    for (int i = 0; i < 3; i++)
    {
        CachedTileset* entry = caches[i];

        while (entry)
        {
            CachedTileset* next = entry->next;

            tileset_free(entry->tileset);
            free(entry);

            entry = next;
        }
    }

    g_regional_cache = NULL;
    g_local_cache    = NULL;
    g_interior_cache = NULL;
}