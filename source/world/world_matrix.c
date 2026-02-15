#include "world/world_matrix.h"
#include <stdlib.h>
#include <string.h>

// Embedded binary
extern const uint8_t _binary_data_world_matrix_mtx_start[] __asm__("_binary_data_world_matrix_mtx_start");
extern const uint8_t _binary_data_world_matrix_mtx_end[] __asm__("_binary_data_world_matrix_mtx_end");

#define MATRIX_MAGIC 0x4D574247     // "GBWM"
#define VERSION 1

void world_matrix_load(WorldMatrix* matrix)
{
    const uint8_t* ptr = _binary_data_world_matrix_mtx_start;

    // Read magic
    uint32_t magic = *(uint32_t*)ptr;
    ptr += sizeof(uint32_t);

    if (magic != MATRIX_MAGIC)
    {
        // Error handling
        return;
    }

    // Read version
    uint16_t version = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    if (version != VERSION)
    {
        // Error handling
        return;
    }

    // Read dimensions
    matrix->width = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    matrix->height = *(uint16_t*)ptr;
    ptr += sizeof(uint16_t);

    // Allocate and copy cells
    size_t cell_count = matrix->width * matrix->height;
    matrix->cells = malloc(cell_count * sizeof(uint16_t));
    memcpy(matrix->cells, ptr, cell_count * sizeof(uint16_t));
}

void world_matrix_free(WorldMatrix* matrix)
{
    if (matrix->cells)
    {
        free(matrix->cells);
        matrix->cells = NULL;
    }
}

uint16_t world_matrix_get(const WorldMatrix* matrix, uint16_t x, uint16_t y)
{
    if (x >= matrix->width || y >= matrix->height)
    {
        return 0;
    }

    return matrix->cells[y * matrix->width + x];
}