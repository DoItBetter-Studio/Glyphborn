#ifndef UI_WIDGETS_H
#define UI_WIDGETS_H

#include "ui/ui_types.h"
#include <stdbool.h>
#include <stdint.h>

bool ui_button(int x, int y, int width, int height, const char* label, uint32_t text_color);
void ui_label(int x, int y, const char* text);
void ui_label_colored(int x, int y, const char* text, uint32_t color);
void ui_bar(int x, int y, int width, int height, float value);
void ui_dialogue_box(int x, int y, int width, int height, const char* text, const uint32_t* portrait);

float ui_scrollbar_vertical(int x, int y, int width, int height, float value);
float ui_scrollbar_horizontal(int x, int y, int width, int height, float value);
UIScrollValue ui_scrollbar_both(int x, int y, int width, int height, UIScrollValue value);

bool ui_item_slot(int x, int y, int size, const uint32_t* item_pixels, int item_width, int item_height);
bool ui_checkbox(int x, int y, int size, bool checked);

void ui_row_highlight(int x, int y, int width, int height);
void ui_panel(int x, int y, int width, int height);
void ui_window(int x, int y, int width, int height, const char* title);
void ui_tooltip(int x, int y, int width, int height, const char* text);
void ui_notification(int x, int y, int width, int height, const char* text);
void ui_cursor(int x, int y);

#endif // !UI_WIDGETS_H