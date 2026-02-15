#include "world/world.h"
#include <stdlib.h>
#include <math.h>

/* External globals, loaded from embedded binary blobs.
 * g_WorldMatrix - WorldMatrix data referenced by world code.
 * g_WorldHeaders - Array of WorldHeader entries with metadata for each map header.
 */
WorldMatrix g_WorldMatrix = {0};
WorldHeaders g_WorldHeaders = {0};

/* Forward declaration for internal loader function */
static void world_load_cell(WorldCell* cell, int mx, int my); 

/**
 * world_init - Initialize the world grid and load initial 3x3 surrounding cells.
 * @world: Pointer to World struct to initialize.
 * @start_x: Center X coordinate.
 * @start_y: Center Y coordinate.
 */
void world_init(World* world, int start_x, int start_y)
{
    world_matrix_load(&g_WorldMatrix);
    world_headers_load(&g_WorldHeaders);

    world->cx = start_x;
    world->cy = start_y; 

    // Load initial 3x3 grid
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            WorldCell* cell = &world->cells[dy+1][dx+1];
            world_load_cell(cell, start_x + dx, start_y + dy);
        }
    }
}

/**
 * world_unload_cell - Free resources associated with a single world cell.
 * @cell: Pointer to the WorldCell to unload.
 *
 * Frees geometry, collision and any loaded tilesets and clears pointers.
 */
static void world_unload_cell(WorldCell* cell)
{
    if (cell->geometry) geometry_free(cell->geometry);
    if (cell->collision) collision_free(cell->collision);
    if (cell->regional_tileset) tileset_free(cell->regional_tileset);
    if (cell->local_tileset) tileset_free(cell->local_tileset);
    if (cell->interior_tileset) tileset_free(cell->interior_tileset);

    cell->geometry = NULL; 
    cell->collision = NULL;
    cell->regional_tileset = NULL;
    cell->local_tileset = NULL;
    cell->interior_tileset = NULL;
}

/**
 * world_load_cell - Load a single world cell at given matrix coords.
 * @cell: Pointer to WorldCell to populate.
 * @mx: Matrix X coordinate.
 * @my: Matrix Y coordinate.
 *
 * Reads header, loads geometry, collision and associated tilesets.
 */
static void world_load_cell(WorldCell* cell, int mx, int my)
{
    uint16_t header_id = world_matrix_get(&g_WorldMatrix, mx, my);
    const WorldHeader* h = world_headers_get(&g_WorldHeaders, header_id);

    cell->header_id = header_id;
    cell->world_x = mx;
    cell->world_y = my;
    cell->vertical_offset = h->vertical_offset; 

    cell->geometry = geometry_load(h->geometry_id);
    cell->collision = collision_load(h->collision_id);

    cell->regional_tileset = tileset_load_regional(h->regional_tileset_id);
    cell->local_tileset = tileset_load_regional(h->local_tileset_id);
    cell->interior_tileset = tileset_load_regional(h->interior_tileset_id);
}

/**
 * world_update - Update world center based on player position and reload cells as needed.
 * @world: Pointer to World instance.
 * @px: Player X position.
 * @pz: Player Z position.
 */
void world_update(World* world, float px, float pz)
{
    int new_cx = (int)floor(px / MAP_WIDTH);
    int new_cy = (int)floor(pz / MAP_HEIGHT);

    if (new_cx == world->cx && new_cy == world->cy)
        return;

    world->cx = new_cx;
    world->cy = new_cy; 

    // Reload 3x3 grid
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            WorldCell* cell = &world->cells[dy+1][dx+1];
            world_unload_cell(cell);
            world_load_cell(cell, new_cx + dx, new_cy + dy);
        }
    }
}

/**
 * world_render - Render the currently loaded 3x3 world cells.
 * @world: Pointer to World instance.
 * @view: View matrix.
 * @projection: Projection matrix.
 */
void world_render(World* world, Mat4 view, Mat4 projection)
{
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            WorldCell* cell = &world->cells[dy+1][dx+1]; 

            Mat4 model = mat4_translate((Vec3){
                dx * MAP_WIDTH,
                cell->vertical_offset,
                dy * MAP_HEIGHT
            });

            render_map(
                cell->geometry,
                cell->regional_tileset,
                cell->local_tileset,
                cell->interior_tileset,
                model,
                view,
                projection
            );
        }
    }
}

/**
 * world_free - Free world resources and unload embedded data.
 * @world: Pointer to World instance.
 */
void world_free(World* world)
{
    for (int y = 0; y < 3; y++)
    { 
        for (int x = 0; x < 3; x++)
        { 
            world_unload_cell(&world->cells[y][x]);
        }
    }

    world_matrix_free(&g_WorldMatrix);
    world_headers_free(&g_WorldHeaders);
}