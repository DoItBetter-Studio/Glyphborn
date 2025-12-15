#ifndef TILEDEF_H
#define TILEDEF_H

#include <stdint.h>

#define TILE_MAX_TYPES  1024

typedef struct TileDef {
    char        name[32];

    uint16_t    mesh_id;
    uint16_t    material_id;

    uint8_t     render_flags;
    uint8_t     collision_type;
    uint8_t     collision_shape_id;

    uint16_t    tag_flags;
    uint16_t    sound_profile;

    uint16_t    lod_mesh_id;
} TileDef;

extern TileDef  g_tile_defs[TILE_MAX_TYPES];
extern uint16_t g_tile_def_count;

void tiledef_load_bin(void);

#endif // TILEDEF_H