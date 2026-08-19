#include "generated/Animations.h"
#include "models/animation.h"
#include "maths/mat4.h"
#include "maths/quat.h"
#include "maths/vec3.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ── Loader ──────────────────────────────────────────────────────────── */

static void parse_track3(const uint8_t** ptr, GbTrack3* t)
{
    t->count  = *(uint16_t*)*ptr; *ptr += 2;
    t->frames = malloc(sizeof(int32_t) * t->count);
    t->values = malloc(sizeof(float)   * t->count * 3);

    memcpy(t->frames, *ptr, sizeof(int32_t) * t->count);
    *ptr += sizeof(int32_t) * t->count;

    memcpy(t->values, *ptr, sizeof(float) * t->count * 3);
    *ptr += sizeof(float) * t->count * 3;
}

static void parse_track4(const uint8_t** ptr, GbTrack4* t)
{
    t->count  = *(uint16_t*)*ptr; *ptr += 2;
    t->frames = malloc(sizeof(int32_t) * t->count);
    t->values = malloc(sizeof(float)   * t->count * 4);

    memcpy(t->frames, *ptr, sizeof(int32_t) * t->count);
    *ptr += sizeof(int32_t) * t->count;

    memcpy(t->values, *ptr, sizeof(float) * t->count * 4);
    *ptr += sizeof(float) * t->count * 4;
}

GbAnimation* parse_animation(const Blob* blob)
{
    const uint8_t* ptr = blob->data;
    const uint8_t* end = blob->data + blob->size;

    uint32_t magic   = *(uint32_t*)ptr; ptr += 4;
    uint16_t version = *(uint16_t*)ptr; ptr += 2;

    if (magic != GBANI_MAGIC || version != GBANI_VERSION)
        return NULL;

    uint16_t clip_count = *(uint16_t*)ptr; ptr += 2;

    GbAnimation* anim = malloc(sizeof(GbAnimation));
    if (!anim) return NULL;

    anim->clip_count    = clip_count;
    anim->clips         = malloc(sizeof(GbClip) * clip_count);
    if (!anim->clips) { free(anim); return NULL; }

    for (uint16_t c = 0; c < clip_count; c++)
    {
        if (ptr + 7 > end) break;

        GbClip* clip        = &anim->clips[c];
        clip->frame_count   = *(int32_t*)ptr;  ptr += 4;
        clip->loop          = *(uint8_t*)ptr;  ptr += 1;
        clip->channel_count = *(uint16_t*)ptr; ptr += 2;
        clip->channels      = malloc(sizeof(GbChannel) * clip->channel_count);
        if (!clip->channels) { gb_animation_free(anim); return NULL; }

        for (uint16_t ch = 0; ch < clip->channel_count; ch++)
        {
            if (ptr + 2 > end) break;
            GbChannel* channel  = &clip->channels[ch];
            channel->bone_index = *(uint16_t*)ptr; ptr += 2;
            parse_track3(&ptr, &channel->position);
            parse_track4(&ptr, &channel->rotation);
            parse_track3(&ptr, &channel->scale);
        }
    }

    return anim;
}

GbAnimation* gb_animation_load(const uint16_t animation_id)
{
    if (animation_id >= g_Animations_Count) return NULL;
    return parse_animation(&g_Animations[animation_id]);
}

void gb_animation_free(GbAnimation* anim)
{
    if (!anim) return;

    for (uint16_t c = 0; c < anim->clip_count; c++)
    {
        GbClip* clip = &anim->clips[c];
        if (!clip->channels) continue;

        for (uint16_t ch = 0; ch < clip->channel_count; ch++)
        {
            GbChannel* channel = &clip->channels[ch];
            free(channel->position.frames); free(channel->position.values);
            free(channel->rotation.frames); free(channel->rotation.values);
            free(channel->scale.frames);    free(channel->scale.values);
        }

        free(clip->channels);
    }

    free(anim->clips);
    free(anim);
}

/* ── Track evaluation ────────────────────────────────────────────────── */

static int32_t find_key(const int32_t* frames, int32_t count, float target)
{
    int32_t lo = 0, hi = count - 1;
    while (lo < hi)
    {
        int32_t mid = (lo + hi + 1) / 2;
        if (frames[mid] <= (int32_t)target) lo = mid;
        else hi = mid - 1;
    }
    return lo;
}

static Vec3 eval_track3(const GbTrack3* t, float frame, Vec3 fallback)
{
    if (t->count == 0) return fallback;

    const float* a;
    const float* b;
    float tf;

    if (frame <= t->frames[0])
    {
        a = &t->values[0];
        return (Vec3){ a[0], a[1], a[2] };
    }

    if (frame >= t->frames[t->count - 1])
    {
        a = &t->values[(t->count - 1) * 3];
        return (Vec3){ a[0], a[1], a[2] };
    }

    int32_t i = find_key(t->frames, t->count, frame);
    tf = (frame - t->frames[i]) / (float)(t->frames[i + 1] - t->frames[i]);
    a  = &t->values[i * 3];
    b  = &t->values[(i + 1) * 3];

    return (Vec3){
        a[0] + (b[0] - a[0]) * tf,
        a[1] + (b[1] - a[1]) * tf,
        a[2] + (b[2] - a[2]) * tf
    };
}

static Quat eval_track4(const GbTrack4* t, float frame)
{
    if (t->count == 0) return quat_identity();

    const float* a;
    const float* b;
    float tf;

    if (frame <= t->frames[0])
    {
        a = &t->values[0];
        return (Quat){ a[0], a[1], a[2], a[3] };
    }

    if (frame >= t->frames[t->count - 1])
    {
        a = &t->values[(t->count - 1) * 4];
        return (Quat){ a[0], a[1], a[2], a[3] };
    }

    int32_t i = find_key(t->frames, t->count, frame);
    tf    = (frame - t->frames[i]) / (float)(t->frames[i + 1] - t->frames[i]);
    a     = &t->values[i * 4];
    b     = &t->values[(i + 1) * 4];

    /* Slerp */
    Quat qa = { a[0], a[1], a[2], a[3] };
    Quat qb = { b[0], b[1], b[2], b[3] };

    float dot = qa.x*qb.x + qa.y*qb.y + qa.z*qb.z + qa.w*qb.w;
    if (dot < 0.0f)
    {
        qb.x = -qb.x; qb.y = -qb.y;
        qb.z = -qb.z; qb.w = -qb.w;
        dot  = -dot;
    }

    if (dot > 0.9995f)
    {
        Quat r = {
            qa.x + (qb.x - qa.x) * tf,
            qa.y + (qb.y - qa.y) * tf,
            qa.z + (qb.z - qa.z) * tf,
            qa.w + (qb.w - qa.w) * tf
        };
        return quat_normalize(r);
    }

    float theta_0 = acosf(dot);
    float theta   = theta_0 * tf;
    float sin_t0  = sinf(theta_0);
    float sa      = sinf(theta_0 - theta) / sin_t0;
    float sb      = sinf(theta)           / sin_t0;

    return (Quat){
        sa * qa.x + sb * qb.x,
        sa * qa.y + sb * qb.y,
        sa * qa.z + sb * qb.z,
        sa * qa.w + sb * qb.w
    };
}

/* ── TRS → Mat4 using Damascus column-major convention ───────────────── */

static Mat4 make_trs(Vec3 pos, Quat rot, Vec3 scl)
{
    float x=rot.x, y=rot.y, z=rot.z, w=rot.w;

    Mat4 m = { 0 };

    m.m[0][0] = (1 - 2*(y*y + z*z)) * scl.x;
    m.m[0][1] = (2*(x*y + z*w))     * scl.x;
    m.m[0][2] = (2*(x*z - y*w))     * scl.x;

    m.m[1][0] = (2*(x*y - z*w))     * scl.y;
    m.m[1][1] = (1 - 2*(x*x + z*z)) * scl.y;
    m.m[1][2] = (2*(y*z + x*w))     * scl.y;

    m.m[2][0] = (2*(x*z + y*w))     * scl.z;
    m.m[2][1] = (2*(y*z - x*w))     * scl.z;
    m.m[2][2] = (1 - 2*(x*x + y*y)) * scl.z;

    /* Translation in column 3 — column-major m[col][row] */
    m.m[3][0] = pos.x;
    m.m[3][1] = pos.y;
    m.m[3][2] = pos.z;
    m.m[3][3] = 1.0f;

    return m;
}

/* ── Evaluator ───────────────────────────────────────────────────────── */

static void compute_world_transform(
    const GbBone* bones,
    const Mat4*   local,
    Mat4*         world,
    bool*         computed,
    uint16_t      bone_count,
    uint16_t      index)
{
    if (computed[index]) return;

    int16_t parent = bones[index].parent;
    if (parent < 0 || parent == (int16_t)index || (uint16_t)parent >= bone_count)
    {
        world[index] = local[index];
        computed[index] = true;
        return;
    }

    compute_world_transform(bones, local, world, computed, bone_count, (uint16_t)parent);
    world[index] = mat4_multiply(world[parent], local[index]);
    computed[index] = true;
}

void gb_animation_evaluate(
    const GbClip*  clip,
    float          frame,
    const GbBone*  bones,
    uint16_t       bone_count,
    Mat4*          palette_out)
{
    Mat4* local    = malloc(sizeof(Mat4) * bone_count);
    Mat4* world    = malloc(sizeof(Mat4) * bone_count);
    bool* computed = calloc(bone_count, sizeof(bool));
    if (!local || !world || !computed) { free(local); free(world); free(computed); return; }

    int16_t* ch_map = malloc(sizeof(int16_t) * bone_count);
    if (!ch_map) { free(local); free(world); free(computed); return; }

    for (uint16_t i = 0; i < bone_count; i++) ch_map[i] = -1;
    for (uint16_t c = 0; c < clip->channel_count; c++)
        if (clip->channels[c].bone_index < bone_count)
            ch_map[clip->channels[c].bone_index] = (int16_t)c;

    /* Pass 1: compute all local transforms first */
    for (uint16_t i = 0; i < bone_count; i++)
    {
        const GbBone* b = &bones[i];

        if (ch_map[i] >= 0)
        {
            const GbChannel* ch = &clip->channels[ch_map[i]];
            Vec3 pos   = eval_track3(&ch->position, frame, b->bind_pos);
            Quat rot   = eval_track4(&ch->rotation, frame);
            Vec3 scale = eval_track3(&ch->scale,    frame, b->bind_scale);
            local[i]   = make_trs(pos, rot, scale);
        }
        else
        {
            local[i] = make_trs(b->bind_pos, b->bind_rot, b->bind_scale);
        }
    }

    /* Pass 2: compute world transforms recursively — order-independent */
    for (uint16_t i = 0; i < bone_count; i++)
        compute_world_transform(bones, local, world, computed, bone_count, i);

    /* Pass 3: skinning palette = world * inv_bind */
    for (uint16_t i = 0; i < bone_count; i++)
        palette_out[i] = mat4_multiply(world[i], bones[i].inv_bind);

    free(ch_map);
    free(computed);
    free(world);
    free(local);
}