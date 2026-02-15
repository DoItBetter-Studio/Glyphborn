#ifndef WORLD_H
#define WORLD_H

#include "world/world_matrix.h"
#include "world/world_headers.h"
#include "world/world_geometry.h"
#include "world/world_collision.h"
#include "world/world_tileset.h"
#include "maths/mat4.h"
#include "render_map.h"
#include <stdint.h>

#define WORLD_RADIUS 1      // 3x3 grid around player

/**
 * WorldCell - Represents a single map cell loaded around the player.
 * @header_id: ID referencing a world header entry.
 * @geometry: Pointer to loaded geometry map (ownership: caller frees via geometry_free).
 * @collision: Pointer to loaded collision map (ownership: caller frees via collision_free).
 * @local_tileset: Pointer to the local tileset for the cell.
 * @regional_tileset: Pointer to the regional tileset for the cell.
 * @interior_tileset: Pointer to the interior tileset for the cell.
 * @world_x, @world_y: Coordinates of this cell in world matrix space.
 * @vertical_offset: Y offset applied when rendering this cell.
 */
typedef struct WorldCell {
    uint16_t header_id;

    GeometryMap* geometry;
    CollisionMap* collision;

    Tileset* local_tileset;
    Tileset* regional_tileset;
    Tileset* interior_tileset;

    int world_x;
    int world_y;
    int16_t vertical_offset;
} WorldCell;

/**
 * World - The active world context centered on the player.
 * @cx, @cy: Current center cell coordinates in world space.
 * @cells: 3x3 grid of loaded `WorldCell` objects (WORLD_RADIUS defines radius).
 */
typedef struct World {
    int cx;     // center map x
    int cy;     // center map y

    WorldCell cells[3][3];  // 3x3 grid
} World; 

// Initialization
/**
 * world_init - Initialize the world grid and load initial cells.
 * @world: Pointer to an allocated World struct to initialize.
 * @start_x: Initial player world X coordinate to center the grid on.
 * @start_y: Initial player world Y coordinate to center the grid on.
 */
void world_init(World* world, int start_x, int start_y);

// Update (player movement)
/**
 * world_update - Update loaded cells based on player position.
 * @world: Pointer to World instance.
 * @player_x: Player X position in world units.
 * @player_z: Player Z position in world units.
 *
 * Recomputes the center cell; if center changes, unloads and reloads
 * the surrounding 3x3 grid to keep the player-centric area populated.
 */
void world_update(World* world, float player_x, float player_z);

// Render
/**
 * world_render - Render all loaded cells.
 * @world: Pointer to World instance.
 * @view: View matrix.
 * @projection: Projection matrix.
 */
void world_render(World* world, Mat4 view, Mat4 projection);

// Cleanup
/**
 * world_free - Free all resources held by the world, including loaded maps and tilesets.
 * @world: Pointer to World instance to free.
 */
void world_free(World* world); 

#endif // !WORLD_H