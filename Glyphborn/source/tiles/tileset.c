#include "tiles/tileset.h"
#include <string.h>

extern const unsigned char _binary_assets_tilesets_bin_start[];
extern const unsigned char _binary_assets_tilesets_bin_end[];

Tileset g_tilesets[TILESET_MAX_COUNT];
uint16_t g_tileset_count = 0;

void tileset_load_bin(void)
{
    const uint8_t* ptr = _binary_assets_tilesets_bin_start;
    
    uint16_t count = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    if (count > TILESET_MAX_COUNT)
        count = TILESET_MAX_COUNT;

    g_tileset_count = count;

    memcpy(g_tilesets, ptr, count * sizeof(Tileset));
}

int tileset_find_by_name(const char* name)
{
    for (uint16_t i = 0; i < g_tileset_count; ++i)
    {
        if (strncmp(g_tilesets[i].name, name, 32) == 0)
            return i;
    }
    return -1;
}