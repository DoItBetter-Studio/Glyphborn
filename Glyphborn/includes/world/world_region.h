#ifndef WORLD_REGION_H
#define WORLD_REGION_H

#include <stdint.h>

#define WORLD_MAX_REGIONS   1024

typedef struct WorldRegion {
    char        name[32];
    uint16_t    regional_tileset;
    uint16_t    exterior_tileset;
    uint16_t    interior_tileset;

    uint32_t    min_chunk_x, min_chunk_z;
    uint32_t    max_chunk_x, max_chunk_z;    
} WorldRegion;

extern WorldRegion  g_regions[WORLD_MAX_REGIONS];
extern uint16_t     g_region_count;

const WorldRegion* world_region_at(uint32_t cx, uint32_t cz);
void world_regions_load_bin(void);

#endif // WORLD_REGION_H