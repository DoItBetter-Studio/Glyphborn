#ifndef WORLD_HEADERS_H
#define WORLD_HEADERS_H

#include <stdint.h>

typedef struct WorldHeader
{
    uint16_t header_id;
    int16_t vertical_offset;
    uint16_t geometry_id;
    uint16_t collision_id;
    uint16_t regional_tileset_id;
    uint16_t local_tileset_id;
    uint16_t interior_tileset_id;
} WorldHeader;

typedef struct WorldHeaders {
    uint16_t count;
    WorldHeader* headers;
} WorldHeaders;

extern WorldHeaders g_WorldHeaders;

void world_headers_load(WorldHeaders* headers);
void world_headers_free(WorldHeaders* headers);
const WorldHeader* world_headers_get(const WorldHeaders* headers, uint16_t header_id);

#endif // !WORLD_HEADER_H