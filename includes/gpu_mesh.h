#ifndef GPU_MESH_H
#define GPU_MESH_H

/*
 * gpu_mesh.h - GPU-side mesh representation for the OpenGL 3.3 pipeline
 *
 * A GPUMesh is created once from a BakedChunk at load time and destroyed
 * when the chunk is unloaded. Drawing is a single glDrawElements call.
 *
 * Vertex layout (interleaved, 32 bytes per vertex):
 *   location 0 : vec3  position  (x, y, z)
 *   location 1 : vec2  texcoord  (u, v)
 *   location 2 : vec3  normal    (nx, ny, nz)
 *
 * Normals are computed per-triangle during upload and written flat (all three
 * vertices of a triangle share the same face normal), giving correct flat
 * shading without a geometry shader.
 *
 * The atlas texture is always ATLAS_SIZE x ATLAS_SIZE BGRA.
 */

#include <stdint.h>
#include "gl_loader.h"

typedef struct GPUVertex
{
    float x,  y,  z;   /* position  */
    float u,  v;        /* texcoord  */
    float nx, ny, nz;   /* face normal */
} GPUVertex;

typedef struct GPUMesh
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint texture;
    uint32_t index_count;
} GPUMesh;

/*
 * gpu_mesh_upload - Build a GPUMesh from a BakedChunk's CPU data.
 *
 * Computes per-face normals from the index+vertex data, builds a new
 * interleaved GPUVertex buffer, and uploads everything to the GPU.
 * The CPU-side BakedChunk data is NOT freed — chunk_free() handles that.
 *
 * Returns 1 on success, 0 on failure.
 */
int gpu_mesh_upload(GPUMesh* mesh,
                    const void*     vertices,   /* RasterVertex* */
                    uint32_t        vertex_count,
                    const uint16_t* indices,
                    uint32_t        index_count,
                    const uint32_t* atlas_pixels,
                    uint32_t        atlas_size);

/*
 * gpu_mesh_free - Delete all GL objects owned by this GPUMesh.
 * Safe to call on a zeroed struct.
 */
void gpu_mesh_free(GPUMesh* mesh);

#endif /* GPU_MESH_H */