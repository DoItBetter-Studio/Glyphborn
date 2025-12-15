#include "world/world_region.h"
#include <string.h>

extern const unsigned char _binary_assets_regions_bin_start[];
extern const unsigned char _binary_assets_regions_bin_end[];

WorldRegion g_regions[WORLD_MAX_REGIONS];
uint16_t    g_region_count = 0;

void world_regions_load_bin(void)
{
    const uint8_t* ptr = _binary_assets_regions_bin_start;

    uint16_t count = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    if (count > WORLD_MAX_REGIONS)
        count = WORLD_MAX_REGIONS;

    g_region_count = count;

    memcpy(g_regions, ptr, count * sizeof(WorldRegion));
}

const WorldRegion* world_region_at(uint32_t cx, uint32_t cz)
{
    const WorldRegion* best = NULL;
    uint64_t best_area = 0;

    for (uint16_t i = 0; i < g_region_count; ++i) {
        const WorldRegion* r = &g_regions[i];

        if (cx < r->min_chunk_x || cx > r->max_chunk_x ||
            cz < r->min_chunk_z || cz > r->max_chunk_z)
            continue;

        uint64_t width  = (uint64_t)(r->max_chunk_x - r->min_chunk_x + 1);
        uint64_t height = (uint64_t)(r->max_chunk_z - r->min_chunk_z + 1);
        uint64_t area   = width * height;

        if (!best || area < best_area) {
            best = r;
            best_area = area;
        }
    }

    return best;
}
