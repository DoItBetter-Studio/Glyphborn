#include "models/mesh.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

GbMesh* gb_mesh_load(const uint8_t* data)
{
    const uint8_t* ptr = data;

    uint32_t magic   = *(uint32_t*)ptr; ptr += 4;
    uint16_t version = *(uint16_t*)ptr; ptr += 2;

    if (magic != GBMSH_MAGIC || version != GBMSH_VERSION)
    {
        fprintf(stderr, "gb_mesh_load: invalid magic or version\n");
        return NULL;
    }

    uint16_t surf_count = *(uint16_t*)ptr; ptr += 2;

    GbMesh* mesh = malloc(sizeof(GbMesh));
    if (!mesh) return NULL;

    mesh->surface_count = surf_count;
    mesh->surfaces      = malloc(sizeof(GbSurface) * surf_count);
    if (!mesh->surfaces) { free(mesh); return NULL; }

    for (uint16_t s = 0; s < surf_count; s++)
    {
        GbSurface* surf = &mesh->surfaces[s];

        surf->vert_count = *(int32_t*)ptr; ptr += 4;
        surf->face_count = *(int32_t*)ptr; ptr += 4;

        surf->verts = malloc(sizeof(GbVertex) * surf->vert_count);
        surf->faces = malloc(sizeof(GbFace)   * surf->face_count);
        if (!surf->verts || !surf->faces) { gb_mesh_free(mesh); return NULL; }

        for (int32_t i = 0; i < surf->vert_count; i++)
        {
            GbVertex* v = &surf->verts[i];
            memcpy(&v->pos,      ptr, sizeof(Vec3));   ptr += sizeof(Vec3);
            memcpy(&v->normal,   ptr, sizeof(Vec3));   ptr += sizeof(Vec3);
            memcpy(&v->uv,       ptr, sizeof(Vec2));   ptr += sizeof(Vec2);
            memcpy(v->bone_idx,  ptr, sizeof(uint16_t) * 4); ptr += sizeof(uint16_t) * 4;
            memcpy(v->bone_wt,   ptr, sizeof(float)    * 4); ptr += sizeof(float)    * 4;
        }

        size_t face_bytes = sizeof(GbFace) * surf->face_count;
        memcpy(surf->faces, ptr, face_bytes);
        ptr += face_bytes;
    }

    return mesh;
}

void gb_mesh_free(GbMesh* mesh)
{
    if (!mesh) return;

    for (uint16_t s = 0; s < mesh->surface_count; s++)
    {
        free(mesh->surfaces[s].verts);
        free(mesh->surfaces[s].faces);
    }

    free(mesh->surfaces);
    free(mesh);
}