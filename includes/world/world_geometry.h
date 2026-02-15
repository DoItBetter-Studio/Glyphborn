#ifndef WORLD_GEOMETRY_H
#define WORLD_GEOMETRY_H

#include <stdint.h>
#include "generated/Geometry.h"

#define GEOMETRY_MAGIC 0x474D4247   // "GBMG"
#define MAP_WIDTH   32
#define MAP_HEIGHT  32
#define MAP_LAYERS  32

typedef struct TileRef {
    uint16_t packed;
} TileRef;

typedef struct GeometryMap {
    TileRef tiles[MAP_LAYERS][MAP_HEIGHT][MAP_WIDTH];
} GeometryMap;

GeometryMap* geometry_load(uint16_t geometry_id);
void geometry_free(GeometryMap* map);

uint8_t tile_get_tileset(TileRef ref);
uint16_t tile_get_id(TileRef ref);

#endif // !WORLD_GEOMETRY_H