#ifndef WORLD_CHUNK_INDEX_H
#define WORLD_CHUNK_INDEX_H

#include <stdint.h>

typedef struct ChunkIndexEntry {
    uint32_t id;
    uint32_t offset;
    uint32_t length;
} ChunkIndexEntry;

extern ChunkIndexEntry* g_chunk_index;
extern uint32_t         g_chunk_index_count;

void chunk_index_load(void);

#endif // WORLD_CHUNK_INDEX_H