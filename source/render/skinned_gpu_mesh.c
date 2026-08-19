#include "render/skinned_gpu_mesh.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Interleaved vertex for the GPU — 14 floats, 56 bytes */
typedef struct
{
    float x,  y,  z;          /* position      */
    float nx, ny, nz;          /* normal        */
    float u,  v;               /* uv            */
    float bi0, bi1, bi2, bi3;  /* bone indices  */
    float bw0, bw1, bw2, bw3;  /* bone weights  */
} SkinnedVertex;

int32_t skinned_gpu_mesh_upload(SkinnedGPUMesh* out, const GbSurface* surf)
{
    if (!out || !surf) return 0;
    if (surf->vert_count == 0 || surf->face_count == 0) return 0;

    memset(out, 0, sizeof(SkinnedGPUMesh));

    /* Build interleaved vertex buffer */
    SkinnedVertex* verts = malloc(sizeof(SkinnedVertex) * surf->vert_count);
    if (!verts) return 0;

    for (int32_t i = 0; i < surf->vert_count; i++)
    {
        const GbVertex* src = &surf->verts[i];
        SkinnedVertex*  dst = &verts[i];

        dst->x  = src->pos.x;    dst->y  = src->pos.y;    dst->z  = src->pos.z;
        dst->nx = src->normal.x; dst->ny = src->normal.y; dst->nz = src->normal.z;
        dst->u  = src->uv.x;     dst->v  = src->uv.y;

        /* Cast uint16_t bone indices to float — shader does int32_t(bi) */
        dst->bi0 = (float)src->bone_idx[0];
        dst->bi1 = (float)src->bone_idx[1];
        dst->bi2 = (float)src->bone_idx[2];
        dst->bi3 = (float)src->bone_idx[3];

        dst->bw0 = src->bone_wt[0];
        dst->bw1 = src->bone_wt[1];
        dst->bw2 = src->bone_wt[2];
        dst->bw3 = src->bone_wt[3];
    }

    /* Build flat uint32_t index buffer from GbFace */
    uint32_t* indices = malloc(sizeof(uint32_t) * surf->face_count * 3);
    if (!indices) { free(verts); return 0; }

    for (int32_t i = 0; i < surf->face_count; i++)
    {
        indices[i * 3 + 0] = (uint32_t)surf->faces[i].a;
        indices[i * 3 + 1] = (uint32_t)surf->faces[i].b;
        indices[i * 3 + 2] = (uint32_t)surf->faces[i].c;
    }

    out->index_count = (uint32_t)(surf->face_count * 3);

    /* Upload VAO / VBO / EBO */
    glGenVertexArrays(1, &out->vao);
    glBindVertexArray(out->vao);

    glGenBuffers(1, &out->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, out->vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(surf->vert_count * sizeof(SkinnedVertex)),
                 verts, GL_STATIC_DRAW);

    free(verts);

    glGenBuffers(1, &out->ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(out->index_count * sizeof(uint32_t)),
                 indices, GL_STATIC_DRAW);

    free(indices);

    /* Vertex attrib layout — must match entity_render_gl.c shader */
    const GLsizei stride = sizeof(SkinnedVertex);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(SkinnedVertex, x));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(SkinnedVertex, nx));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(SkinnedVertex, u));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(SkinnedVertex, bi0));

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(SkinnedVertex, bw0));

    glBindVertexArray(0);

    return 1;
}

void skinned_gpu_mesh_free(SkinnedGPUMesh* mesh)
{
    if (!mesh) return;

    if (mesh->vao && glDeleteVertexArrays)
        glDeleteVertexArrays(1, &mesh->vao);

    if ((mesh->vbo || mesh->ebo) && glDeleteBuffers)
    {
        GLuint bufs[2] = { mesh->vbo, mesh->ebo };
        glDeleteBuffers(2, bufs);
    }

    memset(mesh, 0, sizeof(SkinnedGPUMesh));
}