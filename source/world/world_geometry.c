#include "world/world_geometry.h"
#include <stdlib.h>
#include <string.h>

GeometryMap* geometry_load(uint16_t geometry_id)
{
    if (geometry_id >= g_Geometry_Count)
    {
        return NULL;
    }

    const Blob* blob = &g_Geometry[geometry_id];
    const uint8_t* ptr = blob->data;

    // Read magic
    uint32_t magic = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    if (magic != GEOMETRY_MAGIC)
    {
        return NULL;
    }

    // Read version
    uint16_t version = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    if (version != 1)
    {
        return NULL;
    }

    // Allocate map
    GeometryMap* map = malloc(sizeof(GeometryMap));

    // Read all tiles
    for (int layer = 0; layer < MAP_LAYERS; layer++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                map->tiles[layer][y][x].packed = *(uint16_t*)ptr;
                ptr += sizeof(uint16_t);
            }
        }
    }
    
    return map;
}

void geometry_free(GeometryMap* map)
{
    if (map) {
        free(map);
    }
}

uint8_t tile_get_tileset(TileRef ref)
{
    return (ref.packed >> 14) & 0x3;
}

uint16_t tile_get_id(TileRef ref)
{
    return ref.packed & 0x3FFF;
}