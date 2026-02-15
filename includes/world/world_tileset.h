#ifndef WORLD_TILESET_H
#define WORLD_TILESET_H

#include <stdint.h>

typedef struct Vertex {
    float x, y, z;
    float u, v;
} Vertex;

typedef struct TileMesh {
    Vertex* vertices;
    uint32_t vertex_count;
    uint16_t* indices;
    uint32_t index_count;
    uint32_t* pixels;
    uint16_t texture_width;
    uint16_t texture_height;
} TileMesh;

typedef struct Tileset {
    TileMesh* tiles;
    uint16_t tile_count;
} Tileset;

Tileset* tileset_load_regional(uint16_t tileset_id);
Tileset* tileset_load_local(uint16_t tileset_id);
Tileset* tileset_load_interior(uint16_t tileset_id);
void tileset_free(Tileset* tileset);

#endif // !WORLD_TILESET_H