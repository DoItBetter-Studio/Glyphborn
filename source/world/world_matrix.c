#include "world/world_matrix.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Embedded binary
extern const uint8_t _binary_data_world_matrix_mtx_start[] __asm__("_binary_data_world_matrix_mtx_start");
extern const uint8_t _binary_data_world_matrix_mtx_end[] __asm__("_binary_data_world_matrix_mtx_end");

#define MATRIX_MAGIC 0x4D574247     // "GBWM"
#define VERSION 2

void world_matrix_load(WorldMatrix* matrix)
{
    const uint8_t* ptr = _binary_data_world_matrix_mtx_start;

    // Read magic
    uint32_t magic = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    if (magic != MATRIX_MAGIC)
    {
        printf("Error: world matrix magic mismatch. Expected 0x%X, got 0x%X\n", MATRIX_MAGIC, magic);
        return;
    }

    // Read version
    uint16_t version = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    if (version != VERSION)
    {
        printf("Warning: world matrix version mismatch. Expected %d, got %d\n", VERSION, version);
        return;
    }

    // Read dimensions
    matrix->width = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    matrix->height = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    // Allocate and copy cells
    size_t cell_count = matrix->width * matrix->height;
    matrix->cells = malloc(cell_count * sizeof(Header));

    for (uint32_t i = 0; i < cell_count; i++)
    {
        Header* header = &matrix->cells[i];

        header->header_id = *(uint32_t*)ptr; ptr += sizeof(uint32_t);
        header->vertical_offset = *(int16_t*)ptr; ptr += sizeof(int16_t);
        header->geometry_id = *(uint16_t*)ptr; ptr += sizeof(uint16_t);
        header->collision_id = *(uint16_t*)ptr; ptr += sizeof(uint16_t);
        header->regional_tileset_id = *(uint16_t*)ptr; ptr += sizeof(uint16_t);
        header->local_tileset_id = *(uint16_t*)ptr; ptr += sizeof(uint16_t);
        header->interior_tileset_id = *(uint16_t*)ptr; ptr += sizeof(uint16_t);
        header->metadata_id = *(uint16_t*)ptr; ptr += sizeof(uint16_t);
    }
}

void world_matrix_free(WorldMatrix* matrix)
{
    if (matrix->cells)
    {
        free(matrix->cells);
        matrix->cells = NULL;
    }
}

const Header* world_matrix_get(const WorldMatrix* matrix, uint16_t x, uint16_t y)
{
    if (x >= matrix->width || y >= matrix->height)
    {
        return NULL;
    }

    return &matrix->cells[y * matrix->width + x];
}