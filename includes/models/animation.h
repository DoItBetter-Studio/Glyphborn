#ifndef ANIMATION_H
#define ANIMATION_H

#include "maths/mat4.h"
#include "models/skeleton.h"
#include "generated/Blob.h"
#include <stdint.h>

#define GBANI_MAGIC   0x4E414247u
#define GBANI_VERSION 1

typedef struct
{
    uint16_t count;
    int32_t* frames;
    float*   values;   /* 3 floats per frame */
} GbTrack3;

typedef struct
{
    uint16_t count;
    int32_t* frames;
    float*   values;   /* 4 floats per frame */
} GbTrack4;

typedef struct
{
    uint16_t bone_index;
    GbTrack3 position;
    GbTrack4 rotation;
    GbTrack3 scale;
} GbChannel;

typedef struct
{
    int32_t    frame_count;
    uint8_t    loop;
    uint16_t   channel_count;
    GbChannel* channels;
} GbClip;

typedef struct
{
    uint16_t clip_count;
    GbClip*  clips;
} GbAnimation;

GbAnimation* parse_animation(const Blob* blob);
GbAnimation* gb_animation_load(const uint16_t data);
void         gb_animation_free(GbAnimation* anim);

void gb_animation_evaluate(
    const GbClip*      clip,
    float              frame,
    const GbBone*      bones,
    uint16_t           bone_count,
    Mat4*              palette_out);

#endif /* ANIMATION_H */