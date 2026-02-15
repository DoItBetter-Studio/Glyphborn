#ifndef WORLD_COLLISION_H
#define WORLD_COLLISION_H

#include <stdint.h>
#include "generated/Collision.h"

#define COLLISION_MAGIC 0x434D4247   // "GBMC"
#define MAP_WIDTH   32
#define MAP_HEIGHT  32
#define MAP_LAYERS  32

typedef struct CollisionMap {
    char tiles[MAP_LAYERS][MAP_HEIGHT][MAP_WIDTH];
} CollisionMap;

CollisionMap* collision_load(uint16_t collision_id);
void collision_free(CollisionMap* map);

#endif // !WORLD_COLLISION_H