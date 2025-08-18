#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <stdint.h>

#include "render.h"
#include "ui_skin.h"

#define MAX_PANELS 16

typedef struct
{
	int x;
	int y;
	int width;
	int height;
} Rect;

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

typedef enum
{
	PANEL_MENU,
	PANEL_MAP,
	PANEL_ASSET_BROWSER,
	PANEL_PALETTE_INSPECTOR,
	PANEL_CONSOLE,
	PANEL_CUSTOM
} UIPanelType;

typedef enum
{
	DOCK_LEFT,
	DOCK_RIGHT,
	DOCK_TOP,
	DOCK_BOTTOM,
	DOCK_CENTER
} UIDockZone;

typedef struct
{
	UIPanelType type;
	UIDockZone zone;
	int width;
	int height;
	bool visible;
} UIPanelLayout;

typedef struct
{
	UIPanelLayout* panels;
	int panel_count;
} UIDockLayout;

typedef struct
{
	UIPanelType type;
	Rect rect;
	bool visible;
} UIPanelResolved;

extern UIDockLayout mapper_layout;
extern UIContext g_ui;

// 2D rendering functions
void ui_draw_image(int x, int y, int width, int height, int depth, const unsigned char* image_data, const unsigned char* palette);
void ui_draw_text(int x, int y, const char* text);
void ui_draw_text_colored(int x, int y, const char* text, uint32_t color);
int ui_text_width(const char* text);
int ui_text_height(const char* text, int max_width);
void ui_draw_nineslice(int dst_x, int dst_y, int dst_w, int dst_h, const unsigned char* pixels, unsigned const char* palette, int depth, int src_w, int src_h, int slice_left, int slice_top, int slice_right, int slice_bottom);

// -------------------------------------
// Frame lifecycle
// -------------------------------------
void ui_begin_frame(int mouse_x, int mouse_y, bool mouse_down, bool nav_activate);
void ui_end_frame(void);
void resolve_layout(const UIDockLayout* layout, int fb_width, int fb_height, UIPanelResolved* out_panels, int max_panels);

// -------------------------------------
// Widgets
// -------------------------------------
void ui_menu(int x, int y, int width, int height);
void ui_panel(const UIPanelResolved* panel);
bool ui_button(int x, int y, int width, int height, const char* label, uint32_t text_color);
#endif // !UI_H
