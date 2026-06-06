#ifndef WORLD_BAKER_H
#define WORLD_BAKER_H

#include "world/world_geometry.h"
#include "world/world_tileset.h"
#include "sketch.h"
#include "gpu_mesh.h"
#include <stdint.h>

#define ATLAS_TILE_SIZE  32
#define ATLAS_COLS       32
#define ATLAS_ROWS       32
#define ATLAS_SIZE       (ATLAS_COLS * ATLAS_TILE_SIZE)  /* 1024x1024 */
#define ATLAS_MAX_TILES  (ATLAS_COLS * ATLAS_ROWS)       /* 1024 slots */

typedef struct BakedChunk
{
    /* CPU-side data retained for collision, tools, and future use */
    RasterVertex* vertices;
    uint32_t      vertex_count;
    uint16_t*     indices;
    uint32_t      index_count;
    uint32_t*     atlas;

    /* GPU-side objects — uploaded once at bake time */
    GPUMesh       gpu;
} BakedChunk;

BakedChunk* chunk_bake(
    const GeometryMap* geo,
    const Tileset* regional,
    const Tileset* local,
    const Tileset* interior);

void chunk_free(BakedChunk* chunk);

#endif /* WORLD_BAKER_H */