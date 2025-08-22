#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include "ui/ui_core.h"

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	int offset_x;
	int offset_y;
	int context_width;
	int context_height;
	Rect rect;
} UIScrollView;

void ui_label(int x, int y, const char* text, uint32_t color);
bool ui_button(int x, int y, int width, int height, const char* label, uint32_t text_color);
bool ui_checkbox(int x, int y, int width, int height, const char* label, bool* value);

void ui_begin_scroll(UIScrollView* view, Rect rect);
void ui_end_scroll(UIScrollView* view);

void ui_begin_vertical_scroll(UIScrollView* view, Rect rect);
void ui_end_vertical_scroll(UIScrollView* view);

void ui_begin_horizontal_scroll(UIScrollView* view, Rect rect);
void ui_end_horizontal_scroll(UIScrollView* view);

#endif // !UI_WIDGETS_H
