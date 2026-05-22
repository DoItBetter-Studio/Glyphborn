#include "world/world_tileset.h"
#include "generated/Tileset_Regional.h"
#include "generated/Tileset_Local.h"
#include "generated/Tileset_Interior.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TILESET_MAGIC 0x53544C47  // "GBTS"

static FILE* debug_log = NULL;

static Tileset* parse_tileset(const Blob* blob)
{
    const uint8_t* ptr = blob->data;
    const uint8_t* end = blob->data + blob->size;

    uint32_t magic = *(uint32_t*)ptr; ptr += sizeof(uint32_t);
    uint16_t version = *(uint16_t*)ptr; ptr += sizeof(uint16_t);
    uint16_t tile_count = *(uint16_t*)ptr; ptr += sizeof(uint16_t);

    if (magic != TILESET_MAGIC || version != 1) {
        return NULL;
    }

    Tileset* tileset = malloc(sizeof(Tileset));
    if (!tileset) return NULL;

    tileset->tile_count = tile_count;
    tileset->tiles = malloc(tile_count * sizeof(TileMesh));
    if (!tileset->tiles) { free(tileset); return NULL; }

    // Initialize tiles to safe defaults
    for (uint16_t i = 0; i < tile_count; ++i) {
        tileset->tiles[i].vertices = NULL;
        tileset->tiles[i].indices  = NULL;
        tileset->tiles[i].pixels   = NULL;
        tileset->tiles[i].vertex_count = 0;
        tileset->tiles[i].index_count  = 0;
        tileset->tiles[i].texture_width = 0;
        tileset->tiles[i].texture_height = 0;
    }

    for (uint16_t i = 0; i < tile_count; i++) {
        TileMesh* tile = &tileset->tiles[i];

        if (ptr + sizeof(uint8_t) > end) {
            break;
        }

        tile->vertex_count = *(uint8_t*)ptr; ptr += sizeof(uint8_t);

        if (tile->vertex_count > 0) {
            size_t vbytes = tile->vertex_count * sizeof(Vertex);
            #if _WIN32 || __linux__
            if (ptr + vbytes > end) { fprintf(debug_log, "OOB vertices\n"); break; }
            #endif
            tile->vertices = malloc(vbytes);

            for (uint8_t v = 0; v < tile->vertex_count; v++) {
                tile->vertices[v].x = *(float*)ptr; ptr += sizeof(float);
                tile->vertices[v].y = *(float*)ptr; ptr += sizeof(float);
                tile->vertices[v].z = *(float*)ptr; ptr += sizeof(float);
                tile->vertices[v].u = *(float*)ptr; ptr += sizeof(float);
                tile->vertices[v].v = *(float*)ptr; ptr += sizeof(float);
            }

            // Indices
            if (ptr + sizeof(uint8_t) > end) break;
            tile->index_count = *(uint8_t*)ptr; ptr += sizeof(uint8_t);

            size_t ibytes = tile->index_count * sizeof(uint16_t);
            if (ptr + ibytes > end) break;
            tile->indices = malloc(ibytes);
            memcpy(tile->indices, ptr, ibytes); ptr += ibytes;

            // Texture dimensions
            if (ptr + 4 > end) break;
            tile->texture_width  = ptr[0] | (ptr[1] << 8);
            tile->texture_height = ptr[2] | (ptr[3] << 8);
            ptr += 4;

            size_t pixel_count = (size_t)tile->texture_width * tile->texture_height;
            if (pixel_count > 0) {
                size_t pbytes = pixel_count * sizeof(uint32_t);
                if (ptr + pbytes > end) {
                    tile->pixels = NULL;
                } else {
                    tile->pixels = malloc(pbytes);
                    memcpy(tile->pixels, ptr, pbytes);
                    ptr += pbytes;
                }
            } else {
                tile->pixels = NULL;
            }

        }
        else {
            // Air tile: read index_count + texture dims to match writer
            if (ptr + sizeof(uint8_t) + sizeof(uint16_t)*2 > end) break;

            tile->index_count   = *(uint8_t*)ptr; ptr += sizeof(uint8_t);
            tile->texture_width = *(uint16_t*)ptr; ptr += sizeof(uint16_t);
            tile->texture_height= *(uint16_t*)ptr; ptr += sizeof(uint16_t);
            tile->pixels = NULL;
        }
    }

    return tileset;
}

Tileset* tileset_load_regional(uint16_t tileset_id)
{
    if (tileset_id >= g_Tileset_Regional_Count) return NULL;
    return parse_tileset(&g_Tileset_Regional[tileset_id]);
}

Tileset* tileset_load_local(uint16_t tileset_id)
{
    if (tileset_id >= g_Tileset_Local_Count) return NULL;
    return parse_tileset(&g_Tileset_Local[tileset_id]);
}

Tileset* tileset_load_interior(uint16_t tileset_id)
{
    if (tileset_id >= g_Tileset_Interior_Count) return NULL;
    return parse_tileset(&g_Tileset_Interior[tileset_id]);
}

void tileset_free(Tileset* tileset)
{
    if (!tileset) return;

    for (int i = 0; i < tileset->tile_count; i++)
    {
        TileMesh* tile = &tileset->tiles[i];
        free(tile->vertices);
        free(tile->indices);
        free(tile->pixels);
    }

    free(tileset->tiles);
    free(tileset);
}