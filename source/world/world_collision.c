#include "world/world_collision.h"
#include <stdlib.h>
#include <string.h>

CollisionMap* collision_load(uint16_t collision_id)
{
    if (collision_id >= g_Collision_Count)
    {
        return NULL;
    }

    const Blob* blob = &g_Collision[collision_id];
    const uint8_t* ptr = blob->data;

    // Read magic
    uint32_t magic = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    if (magic != COLLISION_MAGIC)
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
    CollisionMap* map = malloc(sizeof(CollisionMap));

    // Read all tiles
    for (int layer = 0; layer < MAP_LAYERS; layer++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                map->tiles[layer][y][x] = *(char*)ptr;
                ptr += sizeof(char);
            }
        }
    }
    
    return map;
}

void collision_free(CollisionMap* map)
{
    if (map) {
        free(map);
    }
}