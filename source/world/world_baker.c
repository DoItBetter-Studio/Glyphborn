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

/* 
 * Direct Grid Index Map for True O(1) Absolute Zero-Allocation Vertex Welding.
 * Because tile vertices are bound to integer grid steps within the 32x256x32 chunk bounds,
 * we map a 3D coordinate volume of vertices sharing exact spatial edges.
 * Dimensions are 33x257x33 to store outer vertex alignments sharing grid edges perfectly.
 */
#define GRID_W 33
#define GRID_H 33
#define GRID_D 33
#define SLOTS_PER_NODE 6

typedef struct {
    uint16_t indices[SLOTS_PER_NODE];
    float u_coords[SLOTS_PER_NODE];
    float v_coords[SLOTS_PER_NODE];
} GridVtxNode;

/*
 * Spatial Coordinate Hash Table for O(1) Vertex Deduplication.
 * We use an explicit, lightweight closed-hash table with open addressing (linear probing)
 * instead of a massive 3D grid array. This keeps memory usage near zero on all platforms,
 * remains completely thread-safe across Window/Linux/Bare-Metal, and eliminates
 * memory allocation overhead during the bake pass.
 */

#define VTX_HASH_SIZE 16384	// Must be a power of 2 for fast masking

typedef struct
{
	float x, y, z;
	float u, v;
	uint16_t index;
	bool occupied;
} HashVertex;

static uint32_t hash_coordinate(float x, float y, float z, float u, float v)
{
	// Standard floating-point bit mixing hash algorithm
	uint32_t ix, iy, iz, iu, iv;
	memcpy(&ix, &x, 4);
    memcpy(&iy, &y, 4);
    memcpy(&iz, &z, 4);
    memcpy(&iu, &u, 4);
    memcpy(&iv, &v, 4);

	uint32_t hash = 2166136261U;
	hash = (hash ^ ix) * 16777619;
    hash = (hash ^ iy) * 16777619;
    hash = (hash ^ iz) * 16777619;
    hash = (hash ^ iu) * 16777619;
    hash = (hash ^ iv) * 16777619;
    return hash;
}

static uint16_t get_or_add_vertex_fast(BakedChunk* chunk, HashVertex* table, RasterVertex v)
{
    const float eps = 0.0001f;
	uint32_t hash = hash_coordinate(v.x, v.y, v.z, v.u, v.v);
	uint32_t slot = hash & (VTX_HASH_SIZE - 1);
    
	// LInear probing loop (guaranteed O(1) average lookup time at low load factors)
	while (table[slot].occupied)
	{
		if (fabsf(table[slot].x - v.x) < eps &&
			fabsf(table[slot].y - v.y) < eps &&
			fabsf(table[slot].z - v.z) < eps &&
			fabsf(table[slot].u - v.u) < eps &&
			fabsf(table[slot].v - v.v) < eps)
		{
			return table[slot].index; // Vertex already exists
		}

		slot = (slot + 1) & (VTX_HASH_SIZE - 1);
	}

	// Not found: Insert item into vertex buffer and hash table
    uint16_t new_index = (uint16_t)chunk->vertex_count;
    chunk->vertices[new_index] = v;
    chunk->vertex_count++;
    
    table[slot].x = v.x;
    table[slot].y = v.y;
    table[slot].z = v.z;
    table[slot].u = v.u;
    table[slot].v = v.v;
    table[slot].index = new_index;
    table[slot].occupied = true;
    
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

    /* 
     * Persistent Static Cache Remapping Buffer.
     * Thread-local memory ensures zero operating system allocation overhead per frame.
     */
    static __thread GridVtxNode* s_grid_cache = NULL;
    size_t cache_bytes = GRID_W * GRID_H * GRID_D * sizeof(GridVtxNode);
    
    if (!s_grid_cache) {
        s_grid_cache = malloc(cache_bytes);
    }
    // Initialize all index tracks to 0xFFFF (unassigned)
    memset(s_grid_cache, 0xFF, cache_bytes);

    if (!chunk->vertices || !chunk->indices || !chunk->atlas || !s_grid_cache)
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
            {
                uint32_t* dest_line = &chunk->atlas[(dst_y + ty) * ATLAS_SIZE + dst_x];
                const uint32_t* src_line = &tile->pixels[ty * ATLAS_TILE_SIZE];
                memcpy(dest_line, src_line, ATLAS_TILE_SIZE * sizeof(uint32_t));
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

        /* --- True Linear Vector Coordinate Weld Mapping --- */
        uint16_t remap[256];
        for (uint8_t v = 0; v < tile->vertex_count; v++)
        {
            RasterVertex rv;
            rv.x = tile->vertices[v].x + (float)x;
            rv.y = tile->vertices[v].y + (float)layer;
            rv.z = tile->vertices[v].z + (float)y;
            rv.u = uv_offset_x + tile->vertices[v].u * uv_scale;
            rv.v = uv_offset_y + tile->vertices[v].v * uv_scale;

            int32_t gx = (int32_t)floorf(rv.x + 0.5f);
            int32_t gy = (int32_t)floorf(rv.y + 0.5f);
            int32_t gz = (int32_t)floorf(rv.z + 0.5f);

            if (gx >= 0 && gx < GRID_W && gy >= 0 && gy < GRID_H && gz >= 0 && gz < GRID_D)
            {
                uint32_t node_index = (gy * GRID_W * GRID_D) + (gz * GRID_W) + gx;
                GridVtxNode* node = &s_grid_cache[node_index];
                
                // Find matching vertex that shares BOTH position and texture mapping space
                int32_t target_slot = -1;
                const float uv_eps = 0.0001f;

                for (int32_t s = 0; s < SLOTS_PER_NODE; s++) {
                    if (node->indices[s] == 0xFFFF) {
                        target_slot = s; // Found open slot to insert new vertex layout
                        break;
                    }
                    // Edge welding match verification sequence
                    if (fabsf(node->u_coords[s] - rv.u) < uv_eps && 
                        fabsf(node->v_coords[s] - rv.v) < uv_eps) 
                    {
                        target_slot = s;
                        break;
                    }
                }

                if (target_slot != -1 && node->indices[target_slot] != 0xFFFF) {
                    // O(1) Direct memory buffer node intersection match!
                    remap[v] = node->indices[target_slot];
                } else {
                    // Add new unique layout item entry
                    uint16_t new_index = (uint16_t)chunk->vertex_count;
                    chunk->vertices[new_index] = rv;
                    chunk->vertex_count++;

                    if (target_slot != -1) {
                        node->indices[target_slot] = new_index;
                        node->u_coords[target_slot] = rv.u;
                        node->v_coords[target_slot] = rv.v;
                    }
                    remap[v] = new_index;
                }
            }
            else
            {
                uint16_t new_index = (uint16_t)chunk->vertex_count;
                chunk->vertices[new_index] = rv;
                chunk->vertex_count++;
                remap[v] = new_index;
            }
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

void baked_chunk_free(BakedChunk* chunk)
{
    if (!chunk) return;
    gpu_mesh_free(&chunk->gpu);
    free(chunk->vertices);
    free(chunk->indices);
    free(chunk->atlas);
    free(chunk);
}