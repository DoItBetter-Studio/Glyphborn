#ifndef SKELETON_H
#define SKELETON_H

#include "maths/mat4.h"
#include "maths/quat.h"
#include "maths/vec3.h"
#include <stdint.h>

#define GBSK_MAGIC   0x4B534247u
#define GBSK_VERSION 1

typedef struct
{
    int16_t parent;
    Vec3    bind_pos;
    Quat    bind_rot;
    Vec3    bind_scale;
    Mat4    inv_bind;
} GbBone;

typedef struct
{
    uint16_t bone_count;
    GbBone*  bones;
} GbSkeleton;

GbSkeleton* gb_skeleton_load(const uint8_t* data);
void        gb_skeleton_free(GbSkeleton* skel);

#endif /* SKELETON_H */