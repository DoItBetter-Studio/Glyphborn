# Render module

This document describes the rendering subsystem (headers in `includes/`, platform implementations in `source/render/`).

## Overview

The renderer manages three framebuffer layers:

- `framebuffer_game`: game-layer pixels
- `framebuffer_ui`: UI-layer pixels (may be semi-transparent)
- `framebuffer`: the final composed framebuffer which is shown to the display

A per-pixel `depthbuffer` is used by the software rasterizer to perform depth testing when drawing 3D geometry.

Each platform provides an implementation of the renderer (currently `render_windows.c` and `render_linux.c`). These platform files define the actual storage for the framebuffers and implement `render_present` to present the final image to the native windowing system.

---

## Functions

- `void render_init(void* platform_context)`
  Initialize the renderer for the platform. `platform_context` is platform-specific (e.g., `HWND` on Windows or an X11 context struct on Linux).

- `uint32_t* render_get_framebuffer(void)`
  Returns a pointer to the final framebuffer for direct access.

- `void render_clear(uint32_t* buffer, uint32_t color)`
  Fills the provided buffer with `color`.

- `void render_blend_ui_over_game(void)`
  Composites `framebuffer_ui` over `framebuffer_game` into `framebuffer` using per-pixel alpha.

- `void render_present(void)`
  Presents the `framebuffer` to the display, applying scaling and letterboxing to maintain aspect ratio.

- `void render_shutdown(void)`
  Cleans up platform resources.

---

## Notes

- The rendering code performs deterministic operations designed to be simple and portable (no external GPU dependencies).
- The `sketch` module provides rasterization (triangle rasterization and textured rendering) using `depthbuffer` to perform depth tests.

If you'd like, I can produce a visual diagram showing the framebuffer pipeline and call order for a frame (update -> draw -> UI -> blend -> present).