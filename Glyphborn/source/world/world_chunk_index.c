#include "world/world_chunk_index.h"
#include <stdlib.h>
#include <string.h>

extern const uint8_t _binary_chunktbl_bin_start[];
extern const uint8_t _binary_chunktbl_bin_end[];

ChunkIndexEntry* g_chunk_index = NULL;
uint32_t         g_chunk_index_count = 0;

void chunk_index_load(void)
{
    const uint8_t* ptr = _binary_chunktbl_bin_start;

    uint32_t count = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    g_chunk_index_count = count;

    g_chunk_index = malloc(count * sizeof(ChunkIndexEntry));
    memcpy(g_chunk_index, ptr, count * sizeof(ChunkIndexEntry));
}