#include "world/world_baker.h"
#include "render/gpu_mesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct
{
    const uint32_t* pixels;
    int32_t         slot;
} AtlasEntry;

// Spatial deduplication: finds an existing vertex within tolerance or appends a new one
static uint16_t get_or_add_vertex(BakedChunk* chunk, RasterVertex v)
{
    const float eps = 0.0001f;

    for (uint32_t i = 0; i < chunk->vertex_count; i++)
    {
        RasterVertex* candidate = &chunk->vertices[i];
        
        // Match ONLY spatial coordinates to force edge welding
        if (fabsf(candidate->x - v.x) < eps &&
            fabsf(candidate->y - v.y) < eps &&
            fabsf(candidate->z - v.z) < eps)
        {
            // Optional: If UVs must match for distinct faces, keep a vertex split
            if (fabsf(candidate->u - v.u) < eps && fabsf(candidate->v - v.v) < eps) {
                return (uint16_t)i;
            }
        }
    }

    uint16_t new_index = (uint16_t)chunk->vertex_count;
    chunk->vertices[new_index] = v;
    chunk->vertex_count++;
    return new_index;
}

BakedChunk* chunk_bake(
    const GeometryMap* geo,
    const Tileset* regional,
    const Tileset* local,
    const Tileset* interior)
{
    if (!geo || !regional || !local) return NULL;

    /* --- Pass 1: count maximum possible vertices and indices --- */
    uint32_t total_verts   = 0;
    uint32_t total_indices = 0;

    for (int32_t layer = 0; layer < MAP_LAYERS; layer++)
    for (int32_t y = 0; y < MAP_HEIGHT; y++)
    for (int32_t x = 0; x < MAP_WIDTH; x++)
    {
        TileRef ref    = geo->tiles[layer][y][x];
        uint8_t  ts_id  = tile_get_tileset(ref);
        uint16_t tile_id = tile_get_id(ref);

        const TileMesh* tile = NULL;
        switch (ts_id)
        {
            case 0: tile = &regional->tiles[tile_id]; break;
            case 1: tile = &local->tiles[tile_id];    break;
            case 2: if (interior) tile = &interior->tiles[tile_id]; break;
            default: continue;
        }

        if (!tile || tile->vertex_count == 0) continue;

        total_verts   += tile->vertex_count;
        total_indices += tile->index_count;
    }

    if (total_verts == 0) return NULL;

    /* --- Allocate baked chunk --- */
    BakedChunk* chunk = malloc(sizeof(BakedChunk));
    if (!chunk) return NULL;

    chunk->vertices     = malloc(total_verts   * sizeof(RasterVertex));
    chunk->indices      = malloc(total_indices * sizeof(uint16_t));
    chunk->atlas        = calloc(ATLAS_SIZE * ATLAS_SIZE, sizeof(uint32_t));
    chunk->vertex_count = 0;
    chunk->index_count  = 0;
    memset(&chunk->gpu, 0, sizeof(GPUMesh));

    if (!chunk->vertices || !chunk->indices || !chunk->atlas)
    {
        free(chunk->vertices);
        free(chunk->indices);
        free(chunk->atlas);
        free(chunk);
        return NULL;
    }

    /* --- Atlas tracker --- */
    AtlasEntry atlas_entries[ATLAS_MAX_TILES];
    int32_t atlas_count = 0;

    /* --- Pass 2: bake tiles --- */
    for (int32_t layer = 0; layer < MAP_LAYERS; layer++)
    for (int32_t y = 0; y < MAP_HEIGHT; y++)
    for (int32_t x = 0; x < MAP_WIDTH; x++)
    {
        TileRef ref    = geo->tiles[layer][y][x];
        uint8_t  ts_id  = tile_get_tileset(ref);
        uint16_t tile_id = tile_get_id(ref);

        const TileMesh* tile = NULL;
        switch (ts_id)
        {
            case 0: tile = &regional->tiles[tile_id]; break;
            case 1: tile = &local->tiles[tile_id];    break;
            case 2: if (interior) tile = &interior->tiles[tile_id]; break;
            default: continue;
        }

        if (!tile || tile->vertex_count == 0 || !tile->pixels) continue;

        /* --- Find or add atlas slot --- */
        int32_t slot = -1;
        for (int32_t i = 0; i < atlas_count; i++)
        {
            if (atlas_entries[i].pixels == tile->pixels)
            {
                slot = atlas_entries[i].slot;
                break;
            }
        }

        if (slot == -1)
        {
            if (atlas_count >= ATLAS_MAX_TILES) continue;

            slot = atlas_count;
            atlas_entries[atlas_count++] = (AtlasEntry){ tile->pixels, slot };

            int32_t atlas_col = slot % ATLAS_COLS;
            int32_t atlas_row = slot / ATLAS_COLS;
            int32_t dst_x     = atlas_col * ATLAS_TILE_SIZE;
            int32_t dst_y     = atlas_row * ATLAS_TILE_SIZE;

            for (int32_t ty = 0; ty < ATLAS_TILE_SIZE; ty++)
            for (int32_t tx = 0; tx < ATLAS_TILE_SIZE; tx++)
            {
                chunk->atlas[(dst_y + ty) * ATLAS_SIZE + (dst_x + tx)] =
                    tile->pixels[ty * ATLAS_TILE_SIZE + tx];
            }
        }

        /* --- UV offsets for this atlas slot --- */
        int32_t atlas_col = slot % ATLAS_COLS;
        int32_t atlas_row = slot / ATLAS_COLS;

        float texel_size = 1.0f / (float)ATLAS_SIZE;
        float half_texel = 0.5f * texel_size;

        float uv_offset_x = ((float)atlas_col / (float)ATLAS_COLS) + half_texel;
        float uv_offset_y = ((float)atlas_row / (float)ATLAS_ROWS) + half_texel;
        float uv_scale    = (1.0f / (float)ATLAS_COLS) - texel_size;

        /* --- Deduplicate vertices & map local indices to global welded indices --- */
        uint16_t remap[256];
        for (uint8_t v = 0; v < tile->vertex_count; v++)
        {
            RasterVertex rv;
            rv.x = tile->vertices[v].x + (float)x;
            rv.y = tile->vertices[v].y + (float)layer;
            rv.z = tile->vertices[v].z + (float)y;
            rv.u = uv_offset_x + tile->vertices[v].u * uv_scale;
            rv.v = uv_offset_y + tile->vertices[v].v * uv_scale;

            remap[v] = get_or_add_vertex(chunk, rv);
        }

        /* --- Append remapped indices --- */
        for (uint8_t i = 0; i < tile->index_count; i++)
        {
            chunk->indices[chunk->index_count++] = remap[tile->indices[i]];
        }
    }

    /* --- Upload to GPU once at load time --- */
    if (!gpu_mesh_upload(&chunk->gpu,
                         chunk->vertices,
                         chunk->vertex_count,
                         chunk->indices,
                         chunk->index_count,
                         chunk->atlas,
                         ATLAS_SIZE))
    {
        fprintf(stderr, "chunk_bake: gpu_mesh_upload failed\n");
    }

    return chunk;
}

void chunk_free(BakedChunk* chunk)
{
    if (!chunk) return;
    gpu_mesh_free(&chunk->gpu);
    free(chunk->vertices);
    free(chunk->indices);
    free(chunk->atlas);
    free(chunk);
}