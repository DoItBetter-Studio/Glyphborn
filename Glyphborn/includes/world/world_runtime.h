#ifndef WORLD_RUNTIME_H
#define WORLD_RUNTIME_H

#include <stdint.h>
#include "world_chunk.h"

typedef struct WorldRuntime {
    WorldChunk* loaded[9];
    uint32_t player_chunk_x;
    uint32_t player_chunk_z;
} WorldRuntime;

void world_runtime_init(WorldRuntime* w);
void world_runtime_update(WorldRuntime* w, float player_x, float player_z);
void world_runtime_shutdown(WorldRuntime* w);

#endif // WORLD_RUNTIME_H