#ifndef WORLD_H
#define WORLD_H

#include "world/world_matrix.h"
#include "world/world_headers.h"
#include "world/world_geometry.h"
#include "world/world_collision.h"
#include "world/world_tileset.h"
#include "world/world_baker.h"
#include "world/world_tileset_cache.h"
#include "maths/mat4.h"
#include <stdint.h>
#include <stdbool.h>

#define WORLD_RADIUS 1      // 3x3 grid around player

// Step 8 Recommendation: Track state transitions cleanly
typedef enum CellState {
    CELL_EMPTY,
    CELL_LOADING,
    CELL_READY
} CellState;

typedef struct WorldCell {
    uint16_t header_id;
    uint16_t regional_tileset_id;
    uint16_t local_tileset_id;
    uint16_t interior_tileset_id;

    GeometryMap* geometry;
    CollisionMap* collision;

    Tileset* local_tileset;
    Tileset* regional_tileset;
    Tileset* interior_tileset;

    int world_x, world_y;
    int16_t vertical_offset;
    Vec3 bounds_min, bounds_max;

    // State tracking
    CellState state;

    BakedChunk* baked; // Optional: Store baked chunk for rendering optimization
} WorldCell;

typedef struct World {
    int cx;     // Center map cell coordinate X
    int cy;     // Center map cell coordinate Y

    // Step 1 Recommendation: Store pointers instead of direct stack allocations
    WorldCell* cells[3][3];  
} World;

void world_init(World* world, int start_x, int start_y);
void world_update(World* world, float player_x, float player_z);
void world_render(World* world, Mat4 view, Mat4 projection);
void world_free(World* world);

#endif // !WORLD_H