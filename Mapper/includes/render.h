#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>
#include <stdbool.h>

extern uint32_t* framebuffer;
extern uint32_t* framebuffer_mapview;
extern uint32_t* framebuffer_ui;

int fb_width;
int fb_height;

void render_init(void* platform_context);
uint32_t* render_get_framebuffer(void);
void render_clear(uint32_t* buffer, uint32_t color);
void render_blend_ui_under_mapview();
void render_present(void);
void render_shutdown(void);

// 3D rendering functions

#endif // !RENDER_H
