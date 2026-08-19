#ifndef WORLD_MATRIX_H
#define WORLD_MATRIX_H

#include <stdint.h>

typedef struct
{
    uint32_t header_id;
    int16_t vertical_offset;
    uint16_t geometry_id;
    uint16_t collision_id;
    uint16_t regional_tileset_id;
    uint16_t local_tileset_id;
    uint16_t interior_tileset_id;

    uint16_t metadata_id;
} Header;


typedef struct {
    uint16_t width;
    uint16_t height;
    Header* cells;
} WorldMatrix;

extern WorldMatrix g_WorldMatrix;

void world_matrix_load(WorldMatrix* matrix);
void world_matrix_free(WorldMatrix* matrix);
const Header* world_matrix_get(const WorldMatrix* matrix, uint16_t x, uint16_t y);

#endif // !WORLD_MATRIX_H