#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	int offset_y;
	int context_height;
} UIScrollView;

void ui_label(int x, int y, const char* text, uint32_t color);
bool ui_button(int x, int y, int width, int height, const char* label, uint32_t text_color);
bool ui_checkbox(int x, int y, int width, int height, const char* label, bool* value);

void ui_begin_scroll(UIScrollView* view, int x, int y, int w, int h);
void ui_end_scroll(UIScrollView* view, int x, int y, int w, int h);

#endif // !UI_WIDGETS_H
