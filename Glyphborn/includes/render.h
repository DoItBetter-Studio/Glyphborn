#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <stdbool.h>
#include "maths/mat4.h"
#include "maths/vec4.h"
#include "mesh.h"

#define FB_WIDTH		480
#define FB_HEIGHT		320
#define MAX_VERTICES	4096

uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_game[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_ui[FB_WIDTH * FB_HEIGHT];

void render_init(void* platform_context);
uint32_t* render_get_framebuffer(void);
void render_clear(uint32_t* buffer, uint32_t color);
void render_blend_ui_over_game();
void render_present(void);
void render_shutdown(void);

// 3D rendering functions
void render_draw_wireframe(RenderMesh *mesh, Mat4 model, Mat4 view, Mat4 projection, uint32_t color);

// 2D rendering functions
void render_draw_image(int x, int y, int width, int height, int depth, const unsigned char* image_data, const unsigned char* palette);
void render_draw_text(int x, int y, const char* text);
void render_draw_text_colored(int x, int y, const char* text, uint32_t color);
void render_draw_rect(int x, int y, int width, int height, uint32_t color);

#endif // !RENDER_H
