#ifndef WORLD_TILESET_CACHE_H
#define WORLD_TILESET_CACHE_H

#include "world/world_tileset.h"
#include <stdint.h>

Tileset* tileset_acquire_regional(uint16_t id);
Tileset* tileset_acquire_local(uint16_t id);
Tileset* tileset_acquire_interior(uint16_t id);

void tileset_release_regional(uint16_t id);
void tileset_release_local(uint16_t id);
void tileset_release_interior(uint16_t id);

void tileset_cache_shutdown(void);

#endif /* WORLD_TILESET_CACHE_H */