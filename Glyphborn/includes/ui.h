#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stdint.h>

#include "render.h"
#include "ui_skin.h"

typedef struct
{
	int mouse_x, mouse_y;
	bool mouse_down;
	bool nav_activate;
	int hot_item;
	int active_item;
	int next_id;
	int focused_id;
	bool nav_mode;
} UIContext;

extern UIContext g_ui;

// 2D rendering functions
void ui_draw_image(int x, int y, int width, int height, int depth, const unsigned char* image_data, const unsigned char* palette);
void ui_draw_text(int x, int y, const char* text);
void ui_draw_text_colored(int x, int y, const char* text, uint32_t color);
int ui_text_width(const char* text);
int ui_text_height(const char* text, int max_width);
void ui_draw_nineslice(int dst_x, int dst_y, int dst_w, int dst_h, const char* pixels, const char* palette, int depth, int src_w, int src_h, int slice_left, int slice_top, int slice_right, int slice_bottom);

// -------------------------------------
// Frame lifecycle
// -------------------------------------
void ui_begin_frame(int mouse_x, int mouse_y, bool mouse_down, bool nav_activate);
void ui_end_frame(void);

// -------------------------------------
// Widgets
// -------------------------------------
bool ui_button(int x, int y, int width, int height, const char* label, uint32_t bg_color, uint32_t text_color);

#endif // !UI_H
