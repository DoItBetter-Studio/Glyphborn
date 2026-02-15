#include "world/world_headers.h"
#include <stdlib.h>
#include <string.h>

extern const uint8_t _binary_data_world_headers_hdr_start[] __asm__("_binary_data_world_headers_hdr_start");
extern const uint8_t _binary_data_world_headers_hdr_end[] __asm__("_binary_data_world_headers_hdr_end");

#define HEADER_MAGIC 0x20484247
#define VERSION 1

void world_headers_load(WorldHeaders* headers)
{
    const uint8_t* ptr = _binary_data_world_headers_hdr_start;

    // Read magic
    uint32_t magic = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    if (magic != HEADER_MAGIC)
    {
        return;
    }

    // Read version
    uint16_t version = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    if (version != VERSION)
    {
        return;
    }

    // Read count
    headers->count = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    // Allocate headers
    headers->headers = malloc(headers->count * sizeof(WorldHeader));

    // Read all headers
    for (int i = 0; i < headers->count; i++)
    {
        WorldHeader* h = &headers->headers[i];

        h->header_id = *(uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        h->vertical_offset = *(int16_t*)ptr;
        ptr += sizeof(int16_t);

        h->geometry_id = *(uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        h->collision_id = *(uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        h->regional_tileset_id = *(uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        h->local_tileset_id = *(uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        h->interior_tileset_id = *(uint16_t*)ptr;
        ptr += sizeof(uint16_t);

        ptr += sizeof(uint16_t);    // Skip reserved
    }
}

void world_headers_free(WorldHeaders* headers) {
    if (headers->headers) {
        free(headers->headers);
        headers->headers = NULL;
    }
}

const WorldHeader* world_headers_get(const WorldHeaders* headers, uint16_t header_id) {
    for (int i = 0; i < headers->count; i++) {
        if (headers->headers[i].header_id == header_id) {
            return &headers->headers[i];
        }
    }
    return NULL;
}