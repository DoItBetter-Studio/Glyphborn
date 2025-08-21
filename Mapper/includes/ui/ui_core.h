#ifndef UI_CORE_H
#define UI_CORE_H

#include <stdint.h>
#include <stdbool.h>

#define MENU_BAR_HEIGHT		30
#define MAX_UI_ELEMENTS		256

typedef struct
{
	int x;
	int y;
	int width;
	int height;
} Rect;

typedef struct
{
	int mouse_x;
	int mouse_y;
	bool mouse_down;
	bool mouse_pressed;
	bool mouse_released;
	bool mouse_consumed;
	int next_id;
	int current_layer;
} UIContext;

typedef struct
{
	bool hover[MAX_UI_ELEMENTS];
	bool click[MAX_UI_ELEMENTS];
} UIState;

extern UIContext g_ui;

// Lifecycle
void ui_begin_frame(int mouse_x, int mouse_y);
void ui_end_frame(void);

// Utilities
bool ui_mouse_inside(int x, int y, int width, int height);
int ui_gen_id(void);
void ui_set_interaction(int id, bool hover, bool click);
void ui_get_interaction(int id, bool* hover, bool* click);
#endif // !UI_CORE_H
