#ifndef TILESET_H
#define TILESET_H

#include <stdint.h>

#define TILESET_MAX_TILES   1024
#define TILESET_MAX_COUNT   512

typedef enum TilesetCategory {
    TILESET_REGION      = 0,
    TILESET_EXTERIOR    = 1,
    TILESET_INTERIOR    = 2
} TilesetCategory;

typedef struct Tileset {
    char            name[32];
    TilesetCategory category;
    uint16_t        tile_count;
    uint16_t        tile_types[TILESET_MAX_TILES];
} Tileset;

extern Tileset  g_tilesets[TILESET_MAX_COUNT];
extern uint16_t g_tileset_count;

int tileset_find_by_name(const char* name);
void tileset_load_bin(void);

#endif // TILESET_H