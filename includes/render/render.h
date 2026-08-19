#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <stdbool.h>

#define FB_WIDTH		640
#define FB_HEIGHT		360
#define MAX_VERTICES	4096

/**
 * Framebuffers used by the renderer.
 * - `framebuffer` is the final composed framebuffer (UI blended over game)
 * - `framebuffer_game` is the game layer framebuffer
 * - `framebuffer_ui` is the UI layer framebuffer
 *
 * Note: These arrays are declared `extern` here and defined in platform-specific
 * implementations (`render_windows.c` / `render_linux.c`).
 */
extern uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];
extern uint32_t framebuffer_game[FB_WIDTH * FB_HEIGHT];
extern uint32_t framebuffer_ui[FB_WIDTH * FB_HEIGHT];

/**
 * Per-pixel depth buffer used by the 3D rasterizer. Defined in platform renderer.
 */
extern float depthbuffer[FB_WIDTH * FB_HEIGHT];

/**
 * render_init - Initialize rendering backend
 * @platform_context: Platform-specific window/context handle (type depends on platform).
 *
 * Prepares platform resources and sets up any native device/context required for
 * presenting the final framebuffer.
 */
void render_init(void* platform_context);

/**
 * render_get_framebuffer - Return pointer to the final framebuffer for direct access
 *
 * Returns a pointer to the framebuffer used by `render_present`.
 */
uint32_t* render_get_framebuffer(void);

/**
 * render_clear - Fill a buffer with a color
 * @buffer: destination buffer to clear
 * @color: 32-bit ARGB color
 */
void render_clear(uint32_t* buffer, uint32_t color);

/**
 * render_blend_ui_over_game - Composite the UI layer over the game layer into `framebuffer`
 *
 * Performs per-pixel alpha blending and writes the result into `framebuffer`.
 */
void render_blend_ui_over_game();

/**
 * render_present - Present the final framebuffer to the platform window
 */
void render_present(void);

/**
 * render_shutdown - Release renderer resources
 */
void render_shutdown(void);

// 3D rendering functions

#endif // !RENDER_H
