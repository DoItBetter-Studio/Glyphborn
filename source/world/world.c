#include "world/world.h"
#include "frustum.h"
#include "world/world_render_gl.h"
#include "lighting/directional_light.h"
#include <stdlib.h>
#include <math.h>

extern DirectionalLight sun;

WorldMatrix g_WorldMatrix = {0};
WorldHeaders g_WorldHeaders = {0};

/* Forward declarations for internal streaming utilities */
static void world_load_cell_at(WorldCell* cell, int mx, int my);
static void world_unload_cell_internal(WorldCell* cell);

static void world_shift_east(World* world);
static void world_shift_west(World* world);
static void world_shift_north(World* world);
static void world_shift_south(World* world);

// Step 2 Recommendation: Persistent Allocation
void world_init(World* world, int start_x, int start_y)
{
    world_matrix_load(&g_WorldMatrix);
    world_headers_load(&g_WorldHeaders);

    world_gl_init();

    world->cx = start_x;
    world->cy = start_y;

    // Allocate persistent memory pointers once. Content rolls dynamically underneath.
    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            world->cells[x][y] = calloc(1, sizeof(WorldCell));
            world->cells[x][y]->state = CELL_EMPTY;
            
            // Prime initial map state values relative to starting focal position
            int cell_wx = start_x + (x - 1);
            int cell_wy = start_y + (y - 1);
            world_load_cell_at(world->cells[x][y], cell_wx, cell_wy);
        }
    }
}

static void world_load_cell_at(WorldCell* cell, int mx, int my)
{
    if (!cell) return;
    
    // Safety check: Don't double load if it matches active state data
    if (cell->state == CELL_READY && cell->world_x == mx && cell->world_y == my) return;

    world_unload_cell_internal(cell);
    cell->state = CELL_LOADING;

    cell->world_x = mx;
    cell->world_y = my;

    uint16_t header_id = world_matrix_get(&g_WorldMatrix, mx, my);
    cell->header_id = header_id;

    const WorldHeader* header = world_headers_get(&g_WorldHeaders, header_id);
    if (!header)
    {
        cell->geometry = NULL;
        cell->collision = NULL;
        cell->local_tileset = NULL;
        cell->regional_tileset = NULL;
        cell->interior_tileset = NULL;
        cell->vertical_offset = 0;
        cell->state = CELL_EMPTY;
        return;
    }

    cell->vertical_offset = header->vertical_offset;
    cell->geometry = geometry_load(header->geometry_id);
    cell->collision = collision_load(header->collision_id);

    // Note: Step 6 of your recommendation suggests moving these to an acquired cache management layout
    cell->regional_tileset_id = header->regional_tileset_id;
    cell->local_tileset_id    = header->local_tileset_id;
    cell->interior_tileset_id = header->interior_tileset_id;

    cell->regional_tileset = tileset_acquire_regional(header->regional_tileset_id);
    cell->local_tileset    = tileset_acquire_local(header->local_tileset_id);
    cell->interior_tileset = tileset_acquire_interior(header->interior_tileset_id);

    cell->baked = chunk_bake(cell->geometry, cell->regional_tileset, cell->local_tileset, cell->interior_tileset);

    float world_x = (float)(mx * MAP_WIDTH);
    float world_z = (float)(my * MAP_HEIGHT);

    cell->bounds_min = (Vec3){
        world_x,
        (float)cell->vertical_offset,
        world_z
    };

    cell->bounds_max = (Vec3){
        world_x + MAP_WIDTH,
        (float)cell->vertical_offset + MAP_LAYERS,
        world_z + MAP_HEIGHT
    };

    // This cell is ready to be parsed by the rasterizer/mesher
    cell->state = CELL_READY;
}

static void world_unload_cell_internal(WorldCell* cell)
{
    if (!cell || cell->state == CELL_EMPTY) return;

    if (cell->geometry)  { geometry_free(cell->geometry);   cell->geometry = NULL; }
    if (cell->collision) { collision_free(cell->collision); cell->collision = NULL; }
    
    if (cell->regional_tileset) { tileset_release_regional(cell->regional_tileset_id); cell->regional_tileset = NULL; }
    if (cell->local_tileset)    { tileset_release_local(cell->local_tileset_id);    cell->local_tileset = NULL; }
    if (cell->interior_tileset) { tileset_release_interior(cell->interior_tileset_id); cell->interior_tileset = NULL; }

    if (cell->baked) { chunk_free(cell->baked); cell->baked = NULL; }

    cell->state = CELL_EMPTY;
}

// Step 3 Recommendation: Directional Sliding Stream Loops
void world_update(World* world, float player_x, float player_z)
{
    // Derive absolute mathematical index boundaries from physical positions 
    int current_cx = (int)floor(player_x / (float)MAP_WIDTH);
    int current_cy = (int)floor(player_z / (float)MAP_HEIGHT);

    // Calculate shifting displacement vectors
    int dx = current_cx - world->cx;
    int dy = current_cy - world->cy;

    // Process shifts loop intervals until coordinates synchronize seamlessly
    while (dx != 0 || dy != 0)
    {
        if (dx > 0) { world_shift_east(world);  world->cx++; dx--; }
        else if (dx < 0) { world_shift_west(world);  world->cx--; dx++; }
        
        if (dy > 0) { world_shift_south(world); world->cy++; dy--; }
        else if (dy < 0) { world_shift_north(world); world->cy--; dy++; }
    }
}

// Step 4 Recommendation: Shifting Row/Col Pointer Ownerships instead of Reallocating
// Internal helper to safely check if coordinates are within the global matrix bounds
static bool is_valid_matrix_coord(int mx, int my)
{
    return (mx >= 0 && mx < g_WorldMatrix.width && 
            my >= 0 && my < g_WorldMatrix.height);
}

static void world_shift_east(World* world)
{
    for (int y = 0; y < 3; y++)
    {
        WorldCell* recycled = world->cells[0][y];
        world->cells[0][y] = world->cells[1][y];
        world->cells[1][y] = world->cells[2][y];
        world->cells[2][y] = recycled;

        int new_wx = world->cx + 2; 
        int new_wy = world->cy + (y - 1);

        // Safeguard edge boundaries cleanly!
        if (is_valid_matrix_coord(new_wx, new_wy)) {
            world_load_cell_at(recycled, new_wx, new_wy);
        } else {
            // Out of bounds (e.g., negative index or past matrix width)—keep it empty
            world_unload_cell_internal(recycled);
            recycled->world_x = new_wx;
            recycled->world_y = new_wy;
            recycled->state = CELL_EMPTY;
        }
    }
}

static void world_shift_west(World* world)
{
    for (int y = 0; y < 3; y++)
    {
        WorldCell* recycled = world->cells[2][y];
        world->cells[2][y] = world->cells[1][y];
        world->cells[1][y] = world->cells[0][y];
        world->cells[0][y] = recycled;

        int new_wx = world->cx - 2;
        int new_wy = world->cy + (y - 1);

        if (is_valid_matrix_coord(new_wx, new_wy)) {
            world_load_cell_at(recycled, new_wx, new_wy);
        } else {
            world_unload_cell_internal(recycled);
            recycled->world_x = new_wx;
            recycled->world_y = new_wy;
            recycled->state = CELL_EMPTY;
        }
    }
}

static void world_shift_south(World* world)
{
    for (int x = 0; x < 3; x++)
    {
        WorldCell* recycled = world->cells[x][0];
        world->cells[x][0] = world->cells[x][1];
        world->cells[x][1] = world->cells[x][2];
        world->cells[x][2] = recycled;

        int new_wx = world->cx + (x - 1);
        int new_wy = world->cy + 2;

        if (is_valid_matrix_coord(new_wx, new_wy)) {
            world_load_cell_at(recycled, new_wx, new_wy);
        } else {
            world_unload_cell_internal(recycled);
            recycled->world_x = new_wx;
            recycled->world_y = new_wy;
            recycled->state = CELL_EMPTY;
        }
    }
}

static void world_shift_north(World* world)
{
    for (int x = 0; x < 3; x++)
    {
        WorldCell* recycled = world->cells[x][2];
        world->cells[x][2] = world->cells[x][1];
        world->cells[x][1] = world->cells[x][0];
        world->cells[x][0] = recycled;

        int new_wx = world->cx + (x - 1);
        int new_wy = world->cy - 2;

        if (is_valid_matrix_coord(new_wx, new_wy)) {
            world_load_cell_at(recycled, new_wx, new_wy);
        } else {
            world_unload_cell_internal(recycled);
            recycled->world_x = new_wx;
            recycled->world_y = new_wy;
            recycled->state = CELL_EMPTY;
        }
    }
}

/*
 * world_render - GPU path replacing the sketch_draw_mesh call.
 * Drop this function into world.c replacing the existing world_render().
 * Also add to world_init: world_gl_init();
 * And to world_free (before world_matrix_free): world_gl_shutdown();
 * And add to includes: #include "world_render_gl.h"
 */

void world_render(World* world, Mat4 view, Mat4 projection)
{
    Mat4 view_projection = mat4_multiply(projection, view);

    Frustum frustum;
    frustum_extract(&frustum, view_projection);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    world_gl_begin_pass();

    for (int y = 0; y < 3; y++)
    for (int x = 0; x < 3; x++)
    {
        WorldCell* cell = world->cells[x][y];
        if (!cell || !cell->baked || cell->state != CELL_READY) continue;
        if (!frustum_aabb_visible(&frustum, cell->bounds_min, cell->bounds_max)) continue;

        Mat4 model = mat4_translate((Vec3){
            (float)(cell->world_x * MAP_WIDTH),
            (float)cell->vertical_offset,
            (float)(cell->world_y * MAP_HEIGHT)
        });

        world_gl_draw(&cell->baked->gpu, model, view, projection, &sun);
    }
}

void world_free(World* world)
{
    world_gl_shutdown();

    for (int y = 0; y < 3; y++)
    {
        for (int x = 0; x < 3; x++)
        {
            if (world->cells[x][y])
            {
                world_unload_cell_internal(world->cells[x][y]);
                free(world->cells[x][y]);
                world->cells[x][y] = NULL;
            }
        }
    }
    world_matrix_free(&g_WorldMatrix);
    world_headers_free(&g_WorldHeaders);
    tileset_cache_shutdown();
}