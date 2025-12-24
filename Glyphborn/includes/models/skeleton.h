#ifndef SKELETON_H
#define SKELETON_H

#include <stdint.h>
#include "maths/vec3.h"
#include "maths/quat.h"
#include "maths/mat4.h"
#include "maths/conversions.h"

#define GBSK_MAGIC      0x4B534247   // "GBSK"
#define GBSK_VERSION    1
#define GBSK_MAX_NAME   32
#define GBSK_NO_PARENT  0xFFFF

typedef struct
{
    uint16_t    parent;
    char        name[32];
    Vec3        location;
    Quat        rotation;
    Vec3        scale;
} GBSK_Bone;

typedef struct
{
    uint32_t    magic;
    uint16_t    version;
    uint16_t    bone_count;
} GBSK_Header;

typedef struct
{
    uint16_t    parent;
    char        name[GBSK_MAX_NAME];
    Mat4        local_bind;
    Mat4        global_bind;
    Mat4        inverse_bind;
} SkeletonBone;

typedef struct
{
    uint16_t        bone_count;
    SkeletonBone*   bones;
} Skeleton;

Skeleton* skeleton_load_from_blob(const uint8_t* data);

#endif // !SKELETON_H