#include "world/world_runtime.h"
#include <stdlib.h>
#include <string.h>

/*
 * Simple 3x3 loader:
 *
 *  [0] [1] [2]
 *  [3] [4] [5]
 *  [6] [7] [8]
 *
 * Center (index 4) is (player_chunk_x, player_chunk_z).
 */

static void world_runtime_unload_all(WorldRuntime* w)
{
    for (int i = 0; i < 9; ++i) {
        if (w->loaded[i]) {
            chunk_unload(w->loaded[i]);
            w->loaded[i] = NULL;
        }
    }
}

static void world_runtime_load_3x3(WorldRuntime* w, uint32_t center_cx, uint32_t center_cz)
{
    int idx = 0;

    for (int dz = -1; dz <= 1; ++dz) {
        for (int dx = -1; dx <= 1; ++dx) {
            uint32_t cx = center_cx + (int32_t)dx;
            uint32_t cz = center_cz + (int32_t)dz;

            WorldChunk* c = chunk_load(cx, cz);
            w->loaded[idx++] = c;
        }
    }

    w->player_chunk_x = center_cx;
    w->player_chunk_z = center_cz;
}

void world_runtime_init(WorldRuntime* w)
{
    if (!w) {
        return;
    }

    memset(w, 0, sizeof(WorldRuntime));
    w->player_chunk_x = (uint32_t)-1; // Sentinel
    w->player_chunk_z = (uint32_t)-1;
}

/*
 * Assumes:
 *   1 world unit == 1 tile
 *   CHUNK_SIZE tiles per chunk in X and Z
 * so chunk index = floor(pos / CHUNK_SIZE).
 */
void world_runtime_update(WorldRuntime* w, float player_x, float player_z)
{
    if (!w) {
        return;
    }

    if (player_x < 0.0f) player_x = 0.0f;
    if (player_z < 0.0f) player_z = 0.0f;

    uint32_t cx = (uint32_t)(player_x / (float)CHUNK_SIZE);
    uint32_t cz = (uint32_t)(player_z / (float)CHUNK_SIZE);

    if (w->player_chunk_x == cx && w->player_chunk_z == cz) {
        // Still in same center chunk; nothing to do
        return;
    }

    // For now: brute-force unload + reload.
    // You can optimize later by reusing overlapping chunks.
    world_runtime_unload_all(w);
    world_runtime_load_3x3(w, cx, cz);
}

void world_runtime_shutdown(WorldRuntime* w)
{
    if (!w) {
        return;
    }

    world_runtime_unload_all(w);
    memset(w, 0, sizeof(WorldRuntime));
}
