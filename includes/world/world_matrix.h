#ifndef WORLD_MATRIX_H
#define WORLD_MATRIX_H

#include <stdint.h>

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t* cells;
} WorldMatrix;

extern WorldMatrix g_WorldMatrix;

void world_matrix_load(WorldMatrix* matrix);
void world_matrix_free(WorldMatrix* matrix);
uint16_t world_matrix_get(const WorldMatrix* matrix, uint16_t x, uint16_t y);

#endif // !WORLD_MATRIX_H