#include "models/material.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

GbMaterial* gb_material_load(const uint8_t* data) {
    const uint8_t* ptr = data;

    uint32_t magic = *(uint32_t*)ptr; ptr += sizeof(uint32_t);
    uint16_t version = *(uint16_t*)ptr; ptr += sizeof(uint16_t);

    if (magic != GBMAT_MAGIC || version != GBMAT_VERSION)
    {
        fprintf(stderr, "Invalid material magic or version: 0x%08X v%d\n", magic, version);
        return NULL;
    }

    uint32_t width = *(uint32_t*)ptr; ptr += sizeof(uint32_t);
    uint32_t height = *(uint32_t*)ptr; ptr += sizeof(uint32_t);

    if (width == 0 || height == 0) {
        fprintf(stderr, "Invalid material dimensions: %dx%d\n", width, height);
        return NULL;
    }

    GbMaterial* material = malloc(sizeof(GbMaterial));
    if (!material) return NULL;

    material->width = width;
    material->height = height;
    material->gpu_handle = 0; // GPU handle will be set later

    size_t pixel_count = (size_t)width * height;
    size_t pixel_bytes = pixel_count * sizeof(uint32_t);

    material->pixels = malloc(pixel_bytes);
    if (!material->pixels) {
        free(material);
        return NULL;
    }

    memcpy(material->pixels, ptr, pixel_bytes);

    return material;
}

void gb_material_free(GbMaterial* material) {
    if (!material) return;
    free(material->pixels);
    free(material);
}