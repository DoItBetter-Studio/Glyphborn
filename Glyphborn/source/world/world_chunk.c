#include "world/world_chunk.h"
#include "world/world_chunk_index.h"
#include <stdlib.h>
#include <string.h>

extern const uint8_t _binary_chunks_dat_start[];

static const ChunkIndexEntry* find_chunk_entry(uint32_t cx, uint32_t cz)
{
    uint32_t id = (cz << 16) | cx;

    for (uint32_t i = 0; i < g_chunk_index_count; i++)
    {
        if (g_chunk_index[i].id == id)
            return &g_chunk_index[i];
    }

    return NULL;    
}

WorldChunk* chunk_load(uint32_t cx, uint32_t cz)
{
    const ChunkIndexEntry* entry = find_chunk_entry(cx, cz);
    if (!entry)
        return NULL;

    const uint8_t* data = _binary_chunks_dat_start + entry->offset;

    WorldChunk* chunk = malloc(sizeof(WorldChunk));
    chunk->cx = cx;
    chunk->cz = cz;

    chunk->tiles        = malloc(sizeof(TileId) * CHUNK_VOL);
    chunk->collision    = malloc(sizeof(uint8_t) * CHUNK_VOL);

    memcpy(chunk->tiles, data, sizeof(TileId) * CHUNK_VOL);
    memcpy(chunk->collision, data + (sizeof(TileId) * CHUNK_VOL), CHUNK_VOL);

    return chunk;
}

void chunk_unload(WorldChunk* chunk)
{
    if (!chunk) return;

    free(chunk->tiles);
    free(chunk->collision);
    free(chunk);
}
