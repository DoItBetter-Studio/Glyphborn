#ifndef SKINNED_GPU_MESH_H
#define SKINNED_GPU_MESH_H

/*
 * skinned_gpu_mesh.h - GPU upload for skeletal mesh data
 *
 * Converts a GbMesh into a VAO ready for the skinned entity renderer.
 * Vertex layout (14 floats = 56 bytes):
 *   location 0 — vec3 position
 *   location 1 — vec3 normal
 *   location 2 — vec2 uv
 *   location 3 — vec4 bone indices (as float, cast to int in shader)
 *   location 4 — vec4 bone weights
 */

#include "render/gl_loader.h"
#include "models/mesh.h"
#include <stdint.h>

typedef struct
{
    GLuint   vao;
    GLuint   vbo;
    GLuint   ebo;
    uint32_t index_count;
} SkinnedGPUMesh;

/* Upload a single GbSurface to the GPU. Returns 1 on success. */
int  skinned_gpu_mesh_upload(SkinnedGPUMesh* out, const GbSurface* surf);
void skinned_gpu_mesh_free(SkinnedGPUMesh* mesh);

#endif /* SKINNED_GPU_MESH_H */