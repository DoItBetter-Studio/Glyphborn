/*
 * gpu_mesh.c - GPU mesh upload and draw for the OpenGL 3.3 pipeline
 *
 * Converts a BakedChunk's flat RasterVertex + uint16_t index arrays into
 * an interleaved GPUVertex buffer with per-face normals, then uploads to
 * a VAO/VBO/EBO. The atlas is uploaded as a GL_RGBA8 texture using GL_BGRA
 * to match the engine's 0xAARRGGBB pixel format.
 */

#include "render/gpu_mesh.h"
#include "render/sketch.h"     /* RasterVertex */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * gpu_mesh_upload
 * ------------------------------------------------------------------------- */
int32_t gpu_mesh_upload(GPUMesh* mesh,
                    const void*     raw_vertices,
                    uint32_t        vertex_count,
                    const uint16_t* indices,
                    uint32_t        index_count,
                    const uint32_t* atlas_pixels,
                    uint32_t        atlas_size)
{
    if (!mesh || !raw_vertices || !indices || !atlas_pixels) return 0;
    if (vertex_count == 0 || index_count == 0) return 0;

    memset(mesh, 0, sizeof(GPUMesh));

    const RasterVertex* verts = (const RasterVertex*)raw_vertices;

    /* --- Build interleaved GPUVertex buffer with per-face normals ------- */
    GPUVertex* gpu_verts = malloc(vertex_count * sizeof(GPUVertex));
    if (!gpu_verts) return 0;

    /* Copy position + UV first, zero normals */
    for (uint32_t i = 0; i < vertex_count; i++)
    {
        gpu_verts[i].x  = verts[i].x;
        gpu_verts[i].y  = verts[i].y;
        gpu_verts[i].z  = verts[i].z;
        gpu_verts[i].u  = verts[i].u;
        gpu_verts[i].v  = verts[i].v;
        gpu_verts[i].nx = 0.0f;
        gpu_verts[i].ny = 0.0f;
        gpu_verts[i].nz = 0.0f;
    }

    /*
     * Compute flat face normals per triangle.
     * Each triangle's three vertices get the same normal (flat shading).
     * This matches the per-triangle light_factor the CPU rasterizer used,
     * but applied in the fragment shader with the live sun direction.
     */
    for (uint32_t i = 0; i + 2 < index_count; i += 3)
    {
        uint16_t i0 = indices[i + 0];
        uint16_t i1 = indices[i + 1];
        uint16_t i2 = indices[i + 2];

        if (i0 >= vertex_count || i1 >= vertex_count || i2 >= vertex_count)
            continue;

        float ax = verts[i1].x - verts[i0].x;
        float ay = verts[i1].y - verts[i0].y;
        float az = verts[i1].z - verts[i0].z;

        float bx = verts[i2].x - verts[i0].x;
        float by = verts[i2].y - verts[i0].y;
        float bz = verts[i2].z - verts[i0].z;

        /* Cross product a × b */
        float nx = ay * bz - az * by;
        float ny = az * bx - ax * bz;
        float nz = ax * by - ay * bx;

        /* Normalise */
        float len = sqrtf(nx*nx + ny*ny + nz*nz);
        if (len > 1e-6f)
        {
            nx /= len;
            ny /= len;
            nz /= len;
        }

        /*
         * OBJ tiles have no guaranteed winding order so the cross product
         * may point inward. Flip any normal whose Y component is negative —
         * all visible terrain faces should have a net upward-facing normal
         * since the camera always looks down from above. This matches the
         * CPU rasterizer behaviour which used face normals for flat lighting
         * and never had inverted shading.
         */
        if (ny < 0.0f) { nx = -nx; ny = -ny; nz = -nz; }
        
        gpu_verts[i0].nx = nx; gpu_verts[i0].ny = ny; gpu_verts[i0].nz = nz;
        gpu_verts[i1].nx = nx; gpu_verts[i1].ny = ny; gpu_verts[i1].nz = nz;
        gpu_verts[i2].nx = nx; gpu_verts[i2].ny = ny; gpu_verts[i2].nz = nz;
    }

    /* --- Upload VAO / VBO / EBO ---------------------------------------- */
    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(vertex_count * sizeof(GPUVertex)),
                 gpu_verts,
                 GL_STATIC_DRAW);

    free(gpu_verts);

    glGenBuffers(1, &mesh->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(index_count * sizeof(uint16_t)),
                 indices,
                 GL_STATIC_DRAW);

    /* layout(location=0) vec3 position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(GPUVertex),
                          (void*)offsetof(GPUVertex, x));

    /* layout(location=1) vec2 texcoord */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(GPUVertex),
                          (void*)offsetof(GPUVertex, u));

    /* layout(location=2) vec3 normal */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
                          sizeof(GPUVertex),
                          (void*)offsetof(GPUVertex, nx));

    glBindVertexArray(0);

    /* --- Upload atlas texture ------------------------------------------ */
    glGenTextures(1, &mesh->texture);
    glBindTexture(GL_TEXTURE_2D, mesh->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 (GLsizei)atlas_size, (GLsizei)atlas_size,
                 0, GL_BGRA, GL_UNSIGNED_BYTE,
                 atlas_pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    mesh->index_count = index_count;
    return 1;
}

/* -------------------------------------------------------------------------
 * gpu_mesh_free
 * ------------------------------------------------------------------------- */
void gpu_mesh_free(GPUMesh* mesh)
{
    if (!mesh) return;

    if (mesh->texture)
    {
        glDeleteTextures(1, &mesh->texture);
        mesh->texture = 0;
    }

    if (mesh->vao && glDeleteVertexArrays)
    {
        glDeleteVertexArrays(1, &mesh->vao);
        mesh->vao = 0;
    }

    if ((mesh->vbo || mesh->ebo) && glDeleteBuffers)
    {
        GLuint bufs[2] = { mesh->vbo, mesh->ebo };
        glDeleteBuffers(2, bufs);
        mesh->vbo = 0;
        mesh->ebo = 0;
    }

    mesh->index_count = 0;
}