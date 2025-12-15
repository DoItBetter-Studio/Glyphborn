#ifndef WORLD_CHUNK_H
#define WORLD_CHUNK_H

#include <stdint.h>
#include "tiles/tileid.h"

#define CHUNK_SIZE 32
#define CHUNK_VOL (CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE)

typedef struct WorldChunk {
    uint32_t    cx, cz;
    TileId*     tiles;
    uint8_t*    collision;
} WorldChunk;

WorldChunk* chunk_load(uint32_t cx, uint32_t cz);
void chunk_unload(WorldChunk* chunk);

#endif // WORLD_CHUNK_H