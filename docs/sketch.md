# Sketch (rasterizer) module

This document describes the `sketch` subsystem: the software rasterizer and debug drawing utilities.

## Overview

The revised sketch module implements a robust CPU-based triangle rasterizer that performs the full transformation pipeline:

1. Model -> View -> Projection (clip-space)
2. Near-plane clipping in clip-space (z > -w)
3. Perspective divide and viewport transform
4. Triangle rasterization with perspective-correct interpolation of UVs and depth testing against `depthbuffer`

It also includes debugging helpers such as UV visualization and a logging hook for troubleshooting.

## Types

- `RasterVertex` - Per-vertex attributes (position and UV)
- `RasterMesh` - Mesh descriptor including vertex/index arrays and texture pixels. The API is read-only (passes const pointers to avoid copying large arrays)

## Public API

- `void sketch_show_uvs(bool showUVs)`
  Toggle UV visualization mode (useful for debugging texture coordinates).

- `void sketch_clear(uint32_t clear_color)`
  Clear the game-layer framebuffer and reset depth buffer to default far value.

- `void sketch_draw_mesh(const RasterMesh* mesh, Mat4 model, Mat4 view, Mat4 projection)`
  Rasterize the given mesh with the supplied transforms. This function performs near-plane clipping, transforms vertices into clip-space, handles interpolation in screen space using perspective-correct formulas, and writes shaded texels into `framebuffer_game`.

## Implementation notes & behavior

- Near-plane clipping is implemented per-triangle in clip-space and produces up to two output triangles when clipping occurs.
- The rasterizer uses barycentric coordinates with perspective-correct UV interpolation (UVs are divided by clip-space w, interpolated, then divided by interpolated 1/w) — this avoids texture swimming and distortion.
- Depth values are mapped from NDC [-1,1] to [0,1] and stored in `depthbuffer`; lower values are closer.
- Texture sampling is nearest-neighbor (point sampling). When `sketch_show_uvs` is enabled, the shader writes a color visualizing (u,v) instead of sampling the texture.
- The rasterizer writes into `framebuffer_game`; the renderer composites UI over it later.
- The module contains a lightweight debug log (`sketch_debug.txt`) opened on first use to aid troubleshooting.

## Notes / Suggestions

- The current implementation is deterministic and intended for correctness and clarity. Optional improvements: bilinear filtering, backface culling, early Z rejection, and SIMD optimizations for performance.

If you'd like, I can also add a small test harness or unit tests that render a known triangle and compare the resulting framebuffer to a golden image for regression testing.