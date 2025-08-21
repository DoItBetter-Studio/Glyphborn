#ifndef UI_LAYOUT_H
#define UI_LAYOUT_H

#include "ui_core.h"

#define MAX_PANELS			16

typedef enum
{
	PANEL_MAP,
	PANEL_MAP_LAYERS,
	PANEL_ASSET_BROWSER,
	PANEL_PALETTE_INSPECTOR,
	PANEL_CONSOLE,
	PANEL_MATRIX,
	PANEL_MAPS,
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
	Rect rect;
	bool visible;
} UIPanelResolved;

typedef void (*PanelDrawFunc)(const UIPanelResolved*);

typedef struct
{
	char* title;
	UIPanelType type;
	UIDockZone zone;
	Rect rect;
	bool visible;
	PanelDrawFunc draw_func;
} UIPanelLayout;

typedef enum
{
	NODE_SPLIT,
	NODE_LEAF
} UIDockNodeType;

typedef enum
{
	SPLIT_HORIZONTAL,
	SPLIT_VERTICAL
} UISplitType;

typedef struct UIDockNode
{
	UIDockNodeType type;
	Rect rect;

	union {
		struct {
			UISplitType split_type;
			float ratio;
			struct UIDockNode* child_a;
			struct UIDockNode* child_b;
		};

		struct {
			UIPanelLayout* panels[MAX_PANELS];
			int panel_count;
			int active_index;
		};
	};
} UIDockNode;

extern UIDockNode* dock_root;

extern int fb_mapview_width;
extern int fb_mapview_height;

void ui_init_default_layout(void);
void ui_resolve_dock_layout(const UIDockNode* node, Rect parent);
void ui_draw_layout();

#endif // !UI_LAYOUT_H
