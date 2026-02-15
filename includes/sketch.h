#ifndef SKETCH_H
#define SKETCH_H

#include <stdint.h>
#include <stdbool.h>
#include <maths/mat4.h>

/**
 * RasterVertex - Per-vertex input for the software rasterizer
 * @x, @y, @z: Object-space or model-space position (caller supplies positions)
 * @u, @v: Texture coordinates in the [0,1] range
 */
typedef struct RasterVertex
{
    float x, y, z;
    float u, v;
} RasterVertex;

/**
 * RasterMesh - CPU-side mesh representation for `sketch_draw_mesh`
 * @vertices: Pointer to array of `RasterVertex` (read-only)
 * @vertex_count: Number of vertices
 * @indices: Triangle index array (3 indices per triangle)
 * @index_count: Number of indices
 * @pixels: Pointer to RGBA32 texture pixel data (read-only)
 * @tex_width, @tex_height: Texture dimensions
 */
typedef struct RasterMesh
{
    const RasterVertex* vertices;
    uint32_t vertex_count;

    const uint16_t* indices;
    uint32_t index_count;

    const uint32_t* pixels;
    uint16_t tex_width;
    uint16_t tex_height;
} RasterMesh;

/**
 * sketch_show_uvs - Toggle a debug mode that visualizes UV coordinates instead of sampling texture
 * @showUVs: true to visualize UVs, false to use normal texture sampling
 */
void sketch_show_uvs(bool showUVs);

/**
 * sketch_clear - Clear the game framebuffer and reset the depth buffer
 * @clear_color: ARGB color used to clear framebuffer_game
 */
void sketch_clear(uint32_t clear_color);

/**
 * sketch_draw_mesh - Rasterize a mesh using model/view/projection matrices
 * @mesh: Mesh to draw (read-only)
 * @model, @view, @projection: Matrices applied in order to vertices before clipping
 *
 * This function performs clip-space near-plane clipping, perspective divide, viewport
 * transform and perspective-correct interpolation of UVs before rasterizing triangles.
 */
void sketch_draw_mesh(const RasterMesh* mesh, Mat4 model, Mat4 view, Mat4 projection);

#endif // !SKETCH_H
