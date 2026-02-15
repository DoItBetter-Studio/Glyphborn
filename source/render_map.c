#include "render_map.h"
#include "sketch.h"

void render_map(
    const GeometryMap* geo,
    const Tileset* regional,
    const Tileset* local,
    const Tileset* interior,
    Mat4 model,
    Mat4 view,
    Mat4 projection
)
{
    for (int layer = 0; layer < MAP_LAYERS; layer++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                TileRef ref = geo->tiles[layer][y][x];

                uint8_t tileset_id  = tile_get_tileset(ref);
                uint16_t tile_id    = tile_get_id(ref);

                const TileMesh* tile = NULL;

                switch (tileset_id)
                {
                    case 0: tile = &regional->tiles[tile_id]; break;
                    case 1: tile = &local->tiles[tile_id]; break;
                    case 2: tile = &interior->tiles[tile_id]; break;
                    default: continue;
                }

                if (!tile || tile->vertex_count == 0)
                    continue;

                RasterVertex rv[tile->vertex_count];

                for (uint32_t i = 0; i < tile->vertex_count; i++)
                {
                    rv[i].x = tile->vertices[i].x;
                    rv[i].y = tile->vertices[i].y;
                    rv[i].z = tile->vertices[i].z;
                    rv[i].u = tile->vertices[i].u;
                    rv[i].v = tile->vertices[i].v;
                }

                RasterMesh rm = {
                    .vertices       = rv,
                    .vertex_count   = tile->vertex_count,
                    .indices        = tile->indices,
                    .index_count    = tile->index_count,
                    .pixels         = tile->pixels,
                    .tex_width      = tile->texture_width,
                    .tex_height     = tile->texture_height
                };

                Mat4 tile_model = mat4_multiply(
                    model,
                    mat4_translate((Vec3) { (float)x, (float)layer, (float)y})
                );

                sketch_draw_mesh(&rm, tile_model, view, projection);
            }
        }
    }
}
