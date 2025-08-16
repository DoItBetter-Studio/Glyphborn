#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stdint.h>

#include "render.h"

#define MAX_UI_ELEMENTS 128

typedef enum
{
	UI_BUTTON,
	UI_LABEL,
	UI_PANEL,
	UI_CUSTOM,
} UIElementType;

struct UIElement;

typedef void (*UIRenderFunc)(struct UIElement* element);
typedef void (*UIUpdateFunc)(struct UIElement* element, float delta_time);
typedef void (*UIInputFunc)(struct UIElement* element, int mouse_x, int mouse_y, bool mouse_down);

typedef struct UIElement
{
	UIElementType type;
	int x, y, width, height;
	bool visible;

	UIRenderFunc render_func;
	UIUpdateFunc update_func;
	UIInputFunc handle_input_func;

	void* user_data;
} UIElement;

typedef struct
{
	const char* text;
	uint32_t bg_color;
	uint32_t fg_color;
	bool pressed;
	bool focused;
	void (*on_click)(void* user_data);
} UIButton;

typedef struct
{
	const char* text;
	uint32_t color;
} UILabel;

typedef struct
{
	uint32_t bg_color;
} UIPanel;

typedef struct
{
	UIElement* elements[MAX_UI_ELEMENTS];
	int count;
	int focused_element;	// Index of the currently focused element
	bool input_locked;		// When UI is active, lock game input
} UISystem;

void ui_init(UISystem* ui);
void ui_add_element(UISystem* ui, UIElement* element);
void ui_handle_navigation(UISystem* ui, int nav_dx, int nav_dy, bool activate);
void ui_update(UISystem* ui, float delta_time, int mouse_x, int mouse_y, bool mouse_pressed, int nav_dx, int nav_dy, bool activate);
void ui_render(UISystem* ui);
void ui_clear(UISystem* ui);

UIElement* ui_create_button(int x, int y, int width, int height, const char* text, uint32_t bg_color, uint32_t fg_color, void (*on_click)(void* user_data));
UIElement* ui_create_label(int x, int y, const char* text, uint32_t color);
UIElement* ui_create_panel(int x, int y, int width, int height, uint32_t bg_color);

#endif // !UI_H
