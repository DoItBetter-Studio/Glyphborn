# World module

This document describes the `world` subsystem (headers in `includes/world/`, implementation in `source/world/`). It includes the public types and function-level documentation for the main API.

## Overview

The world system manages a small set of map "cells" around the player's current position. It uses two embedded binary blobs:

- `WorldMatrix` - a matrix of header IDs for world coordinates
- `WorldHeaders` - metadata entries (geometry, collision, tileset IDs, vertical offsets)

At runtime the engine keeps a 3x3 grid (WORLD_RADIUS == 1) centered on the player, loading and unloading cells deterministically when the player crosses cell boundaries.

---

## Types

### `WorldCell`
- `header_id` (uint16_t): header id from `WorldHeaders`
- `geometry` (GeometryMap*): loaded geometry for the cell
- `collision` (CollisionMap*): loaded collision data
- `local_tileset`, `regional_tileset`, `interior_tileset` (Tileset*): tilesets used for rendering
- `world_x`, `world_y` (int): coordinates in matrix space
- `vertical_offset` (int16_t): offset applied when rendering

### `World`
- `cx`, `cy` (int): center cell coordinates
- `cells[3][3]` (WorldCell): loaded 3x3 window

---

## Functions

### `void world_init(World* world, int start_x, int start_y)`
Initialize the world context and load the initial 3x3 grid centered on `(start_x, start_y)`.

### `void world_update(World* world, float player_x, float player_z)`
Recompute center cell from player position and reload cells when the center changes.

### `void world_render(World* world, Mat4 view, Mat4 projection)`
Render the currently loaded cells, applying each cell's vertical offset.

### `void world_free(World* world)`
Free resources for all loaded cells and free embedded matrices/headers.

---

## Notes & Implementation details

- All data is loaded deterministically from embedded binary blobs (see `world_matrix` and `world_headers`).
- The world subsystem expects ownership semantics: callers allocate `World` and the subsystem uses helper functions like `geometry_free`, `collision_free`, and `tileset_free` to release resources.

---

If you want this rendered into a docs site or converted to HTML/MkDocs format, I can add `mkdocs.yml` and the necessary structure next.