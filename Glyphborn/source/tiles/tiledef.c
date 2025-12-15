#include "tiles/tiledef.h"
#include <string.h>
#include <stdint.h>

extern const unsigned char _binary_assets_tiledefs_bin_start[];
extern const unsigned char _binary_assets_tiledefs_bin_end[];

TileDef     g_tile_defs[TILE_MAX_TYPES];
uint16_t    g_tile_def_count = 0;

void tiledef_load_bin(void)
{
    const uint8_t* ptr = _binary_assets_tiledefs_bin_start;

    uint16_t count = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    if (count > TILE_MAX_TYPES)
        count = TILE_MAX_TYPES;
    
    g_tile_def_count = count;

    memcpy(g_tile_defs, ptr, count * sizeof(TileDef));
}