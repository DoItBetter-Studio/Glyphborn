#ifndef RENDER_MAP_H
#define RENDER_MAP_H

#include "world/world_geometry.h"
#include "world/world_tileset.h"
#include "maths/mat4.h"

void render_map(
    const GeometryMap* geo,
    const Tileset* regional,
    const Tileset* local,
    const Tileset* interior,
    Mat4 model,
    Mat4 view,
    Mat4 projection
);

#endif // !RENDER_MAP_H