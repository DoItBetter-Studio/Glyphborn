#ifndef MESH_H
#define MESH_H

#include "maths/vec2.h"
#include "maths/vec3.h"
#include <stdint.h>

#define GBMSH_MAGIC   0x534D4247u
#define GBMSH_VERSION 1

typedef struct
{
    Vec3     pos;
    Vec3     normal;
    Vec2     uv;
    uint16_t bone_idx[4];
    float    bone_wt[4];
} GbVertex;

typedef struct
{
    int32_t a, b, c;
} GbFace;

typedef struct
{
    int32_t   vert_count;
    int32_t   face_count;
    GbVertex* verts;
    GbFace*   faces;
} GbSurface;

typedef struct
{
    uint16_t   surface_count;
    GbSurface* surfaces;
} GbMesh;

GbMesh* gb_mesh_load(const uint8_t* data);
void    gb_mesh_free(GbMesh* mesh);

#endif /* MESH_H */