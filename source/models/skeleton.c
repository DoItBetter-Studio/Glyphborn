#include "models/skeleton.h"
#include "generated/Skeletons.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

GbSkeleton* gb_skeleton_load(const uint8_t* data)
{
    const uint8_t* ptr = data;

    uint32_t magic   = *(uint32_t*)ptr; ptr += 4;
    uint16_t version = *(uint16_t*)ptr; ptr += 2;

    if (magic != GBSK_MAGIC || version != GBSK_VERSION)
    {
        fprintf(stderr, "gb_skeleton_load: invalid magic or version\n");
        return NULL;
    }

    uint16_t bone_count = *(uint16_t*)ptr; ptr += 2;

    GbSkeleton* skel = malloc(sizeof(GbSkeleton));
    if (!skel) return NULL;

    skel->bone_count = bone_count;
    skel->bones      = malloc(sizeof(GbBone) * bone_count);
    if (!skel->bones) { free(skel); return NULL; }

    for (uint16_t i = 0; i < bone_count; i++)
    {
        GbBone* b = &skel->bones[i];

        b->parent = *(int16_t*)ptr; ptr += 2;

        memcpy(&b->bind_pos,   ptr, sizeof(Vec3)); ptr += sizeof(Vec3);
        memcpy(&b->bind_rot,   ptr, sizeof(Quat)); ptr += sizeof(Quat);
        memcpy(&b->bind_scale, ptr, sizeof(Vec3)); ptr += sizeof(Vec3);
        memcpy(&b->inv_bind,   ptr, sizeof(Mat4)); ptr += sizeof(Mat4);
    }

    return skel;
}

void gb_skeleton_free(GbSkeleton* skel)
{
    if (!skel) return;
    free(skel->bones);
    free(skel);
}