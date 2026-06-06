/*
 * gpu_mesh.c - GPU mesh upload and draw for the OpenGL 3.3 pipeline
 *
 * Converts a BakedChunk's flat RasterVertex + uint16_t index arrays into
 * an interleaved GPUVertex buffer with per-face normals, then uploads to
 * a VAO/VBO/EBO. The atlas is uploaded as a GL_RGBA8 texture using GL_BGRA
 * to match the engine's 0xAARRGGBB pixel format.
 */

#include "gpu_mesh.h"
#include "sketch.h"     /* RasterVertex */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

/* -------------------------------------------------------------------------
 * GL enumerants not in gl_loader.h (texture + draw path)
 * These are all GL 1.x/2.x values available without proc loading on both
 * platforms, but we define them here to avoid pulling in glext.h.
 * ------------------------------------------------------------------------- */
#define GL_ELEMENT_ARRAY_BUFFER     0x8893
#define GL_STATIC_DRAW              0x88E4
#define GL_RGBA8                    0x8058
#define GL_BGRA                     0x80E1
#define GL_UNSIGNED_BYTE            0x1401
#define GL_TEXTURE_2D               0x0DE1
#define GL_TEXTURE_MIN_FILTER       0x2801
#define GL_TEXTURE_MAG_FILTER       0x2800
#define GL_TEXTURE_WRAP_S           0x2802
#define GL_TEXTURE_WRAP_T           0x2803
#define GL_NEAREST                  0x2600
#define GL_CLAMP_TO_EDGE            0x812F
#define GL_TRIANGLES                0x0004
#define GL_UNSIGNED_SHORT           0x1403
#define GL_FLOAT                    0x1406
#define GL_FALSE                    0

/* -------------------------------------------------------------------------
 * glDeleteBuffers / glDeleteVertexArrays — loaded locally since they are
 * not in gl_loader.h (only needed here and in render shutdown).
 * ------------------------------------------------------------------------- */
typedef void (*PFNGLDELETEBUFFERSPROC_LOCAL)      (GLsizei n, const GLuint* buffers);
typedef void (*PFNGLDELETEVERTEXARRAYSPROC_LOCAL) (GLsizei n, const GLuint* arrays);

static PFNGLDELETEBUFFERSPROC_LOCAL      s_glDeleteBuffers      = NULL;
static PFNGLDELETEVERTEXARRAYSPROC_LOCAL s_glDeleteVertexArrays = NULL;

static void ensure_delete_procs(void)
{
    if (!s_glDeleteBuffers)
    {
#ifdef _WIN32
        s_glDeleteBuffers      = (PFNGLDELETEBUFFERSPROC_LOCAL)      wglGetProcAddress("glDeleteBuffers");
        s_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC_LOCAL) wglGetProcAddress("glDeleteVertexArrays");
#else
        s_glDeleteBuffers      = (PFNGLDELETEBUFFERSPROC_LOCAL)      glXGetProcAddress((const GLubyte*)"glDeleteBuffers");
        s_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC_LOCAL) glXGetProcAddress((const GLubyte*)"glDeleteVertexArrays");
#endif
    }
}

/* -------------------------------------------------------------------------
 * gpu_mesh_upload
 * ------------------------------------------------------------------------- */
int gpu_mesh_upload(GPUMesh* mesh,
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

    
    // After the normal loop in gpu_mesh_upload, add temporarily:
    for (uint32_t i = 0; i + 2 < index_count; i += 3) {
        float nx = gpu_verts[indices[i]].nx;
        float ny = gpu_verts[indices[i]].ny;
        float nz = gpu_verts[indices[i]].nz;
        printf("tri%u normal: %.3f %.3f %.3f\n", i/3, nx, ny, nz);
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

    ensure_delete_procs();

    if (mesh->texture)
    {
        glDeleteTextures(1, &mesh->texture);
        mesh->texture = 0;
    }

    if (mesh->vao && s_glDeleteVertexArrays)
    {
        s_glDeleteVertexArrays(1, &mesh->vao);
        mesh->vao = 0;
    }

    if ((mesh->vbo || mesh->ebo) && s_glDeleteBuffers)
    {
        GLuint bufs[2] = { mesh->vbo, mesh->ebo };
        s_glDeleteBuffers(2, bufs);
        mesh->vbo = 0;
        mesh->ebo = 0;
    }

    mesh->index_count = 0;
}