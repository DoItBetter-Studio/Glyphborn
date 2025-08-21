#ifndef UI_RENDER_H
#define UI_RENDER_H

#include <stdint.h>

void ui_draw_image(int x, int y, int width, int height,	const unsigned char* image_data, const unsigned char* palette, int depth);
void ui_draw_text(int x, int y, const char* text);
void ui_draw_text_colored(int x, int y, const char* text, uint32_t color);

int ui_text_width(const char* text);
int ui_text_height(const char* text, int max_width);

void ui_draw_nineslice(int dst_x, int dst_y, int dst_w, int dst_h, const unsigned char* image_data, const unsigned char* palette, int depth, int src_w, int src_h, int slice_left, int slice_top, int slice_right, int slice_bottom);

void ui_set_clip(int x, int y, int width, int height);
void ui_set_transform(int tx, int ty);

#endif // !UI_RENDER_H
