#ifndef UI_CORE_H
#define UI_CORE_H

#include <stdint.h>
#include <stdbool.h>

#define MENU_BAR_HEIGHT				30
#define MAX_UI_ELEMENTS				256
#define UI_CLIP_STACK_MAX			16
#define UI_TRANSFORM_STACK_MAX		16

typedef struct
{
	int x;
	int y;
	int width;
	int height;
} Rect;

typedef struct
{
	int tx;
	int ty;
} UITransform;

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

	Rect clip_stack[UI_CLIP_STACK_MAX];
	int clip_top;
	Rect current_clip;

	UITransform transform_stack[UI_TRANSFORM_STACK_MAX];
	int transform_top;
	UITransform current_transform;
} UIContext;

typedef struct
{
	bool hover[MAX_UI_ELEMENTS];
	bool click[MAX_UI_ELEMENTS];
} UIState;

extern UIContext g_ui;
extern Rect g_clip;
extern int g_tx;
extern int g_ty;

// Lifecycle
void ui_begin_frame();
void ui_end_frame(void);

// Utilities
bool ui_mouse_inside(int x, int y, int width, int height);
int ui_gen_id(void);
void ui_set_interaction(int id, bool hover, bool click);
void ui_get_interaction(int id, bool* hover, bool* click);

void ui_push_clip(int x, int y, int width, int height);
void ui_pop_clip(void);
void ui_translate(int dx, int dy);
void ui_pop_transform(void);
#endif // !UI_CORE_H
