#ifndef MATERIAL_H
#define MATERIAL_H

#include <stdint.h>

#define GBMAT_MAGIC 0x544D4247  // "GBMT"
#define GBMAT_VERSION 1

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t* pixels; // RGBA8
    uint32_t gpu_handle;
} GbMaterial;

GbMaterial* gb_material_load(const uint8_t* data);
void gb_material_free(GbMaterial* material);

#endif // !MATERIAL_H