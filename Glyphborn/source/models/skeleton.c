#include "models/skeleton.h"
#include <stdlib.h>

Skeleton* skeleton_load_from_blob(const uint8_t* data)
{
    const GBSK_Header* hdr = (const GBSK_Header*)data;

    if (hdr->magic != GBSK_MAGIC)
        return NULL;
    
    if (hdr->version != GBSK_VERSION)
        return NULL;

    const GBSK_Bone* src = (const GBSK_Bone*)(hdr + 1);

    Skeleton* skel = malloc(sizeof(Skeleton));
    skel->bone_count = hdr->bone_count;
    skel->bones = calloc(hdr->bone_count, sizeof(SkeletonBone));

    // Build local matrices
    for (uint16_t i = 0; i < hdr->bone_count; i++)
    {
        skel->bones[i].parent = src[i].parent;
        skel->bones[i].local_bind = mat4_from_trs(src[i].location, src[i].rotation, src[i].scale);
    }

    // Build global + inverse bind
    for (uint16_t i = 0; i < hdr->bone_count; i++)
    {
        if (skel->bones[i].parent == GBSK_NO_PARENT)
        {
            skel->bones[i].global_bind = skel->bones[i].local_bind;
        }
        else
        {
            uint16_t p = skel->bones[i].parent;
            skel->bones[i].global_bind = mat4_multiply(skel->bones[p].global_bind, skel->bones[i].local_bind);
        }

        skel->bones[i].inverse_bind = mat4_inverse_affine(skel->bones[i].global_bind);
    }

    return skel;
}