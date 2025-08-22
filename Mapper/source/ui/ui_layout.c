#include "ui/ui_layout.h"
#include "render.h"
#include "ui/ui_skin.h"
#include "ui/ui_render.h"
#include "ui/ui_widgets.h"
#include "sketch.h"
#include "camera.h"
#include "input.h"
#include "test_tile.h"
#include <stdio.h>

UIDockNode* dock_root;

UIDockNode* make_leaf(UIPanelLayout* panel)
{
	UIDockNode* node = calloc(1, sizeof(UIDockNode));
	node->type = NODE_LEAF;
	node->panels[0] = panel;
	node->panel_count = 1;
	node->active_index = 0;
	return node;
}

UIDockNode* make_split(UISplitType type, float ratio, UIDockNode* a, UIDockNode* b)
{
	UIDockNode* node = calloc(1, sizeof(UIDockNode));
	node->type = NODE_SPLIT;
	node->split_type = type;
	node->ratio = ratio;
	node->child_a = a;
	node->child_b = b;
	return node;
}

static void ui_panel(const UIPanelResolved* panel)
{
	if (!panel) return;

	const UISkin* skin = ui_get_skin();

	ui_draw_nineslice(
		panel->rect.x,
		panel->rect.y,
		panel->rect.width,
		panel->rect.height,
		skin->panel.pixels,
		skin->active_palette,
		skin->panel.depth,
		skin->panel.width,
		skin->panel.height,
		4, 4, 4, 4
	);
}

int fb_mapview_width = 0;
int fb_mapview_height = 0;
Camera main_camera = { 0 };
Mat4 view;
Mat4 projection;

bool once = false;

RenderMesh cube_mesh = {
	.vertices = cube_vertices,
	.normals = NULL,
	.uvs = NULL,
	.indices = cube_indices,
	.vertex_count = 8,
	.index_count = 36,
	.material_ids = NULL,
	.material_count = 0,
	.texture_data = NULL,
	.texture_width = 0,
	.texture_height = 0,
	.bounds_min = {-1.0, -1.0, -1.0},
	.bounds_max = {1.0, 1.0, 1.0}
};

static void ui_map_panel(const UIPanelResolved* panel)
{
	int id = ui_gen_id();

	bool hover, click;
	ui_get_interaction(id, &hover, &click);

	if (!panel)	return;

	if (!once)
	{
		main_camera.position = (Vec3){ 0.0f, 30.0f, -30.0f };
		main_camera.target = (Vec3){ 0.0f, 0.0f, 0.0f };
		main_camera.up = (Vec3){ 0.0f, 1.0f, 0.0f };

		once = true;
	}

	view = camera_get_view_matrix(&main_camera);
	projection = mat4_perspective(3.14159f / 4.0f, (float)fb_mapview_width / (float)fb_mapview_height, 0.1f, 100.0f);

	ui_panel(panel);

	for (int i = 0; i < fb_mapview_width * fb_mapview_height; ++i)
	{
		framebuffer_mapview[i] = 0xFFAAAAAA;
		depthbuffer_mapview[i] = 1e9f;
	}

	int w = panel->rect.width - 8;
	int h = panel->rect.height - 8;

	if (w > 0 && h > 0 && (w != fb_mapview_width || h != fb_mapview_height))
	{
		if (framebuffer_mapview) free(framebuffer_mapview);
		if (depthbuffer_mapview) free(depthbuffer_mapview);

		framebuffer_mapview = (uint32_t*)calloc(w * h, sizeof(uint32_t));
		depthbuffer_mapview = (float*)malloc(w * h * sizeof(float));

		fb_mapview_width = w;
		fb_mapview_height = h;
	}

	if (fb_mapview_width == 0 || fb_mapview_height == 0) return;

	sketch_draw_grid(32, g_ui.current_layer, 32, view, projection);

	sketch_draw_wireframe(&cube_mesh, mat4_identity(), view, projection, 0xFF000000);

	for (int y = panel->rect.y + 4; y < panel->rect.y + panel->rect.height - 4; ++y)
	{
		for (int x = panel->rect.x + 4; x < panel->rect.x + panel->rect.width - 4; ++x)
		{
			int local_x = x - (panel->rect.x + 4);
			int local_y = y - (panel->rect.y + 4);
			framebuffer_ui[y * fb_width + x] = framebuffer_mapview[local_y * fb_mapview_width + local_x];
		}
	}

	if (!g_ui.mouse_consumed)
	{
		ui_set_interaction(id, hover, click);
	}
}

static void ui_layers_panel(const UIPanelResolved* panel)
{
	if (!panel)	return;

	ui_panel(panel);

	Rect rect = panel->rect;

	for (int i = 0; i < 8; ++i)
	{
		char buffer[100];
		snprintf(buffer, sizeof(buffer), "Label %d", i + 1);
		uint32_t color = g_ui.current_layer == i ? 0xFF00FF00 : 0xFFFFFFFF;
		if (ui_button(rect.x + 8, rect.y + 8 + (i * (32 + 4)), rect.width - 16, 32, buffer, color))
		{
			g_ui.current_layer = i;
		}
	}
}

bool checked[32];

UIScrollView asset_scroll_view;

static void ui_asset_panel(const UIPanelResolved* panel)
{
	if (!panel)	return;

	ui_panel(panel);

	Rect rect = panel->rect;

	ui_begin_vertical_scroll(&asset_scroll_view, (Rect) { rect.x + 4, rect.y + 4, rect.width - 8, rect.height - 8 });

	for (int i = 0; i < 32; ++i)
	{
		checked[i] = ui_checkbox(rect.x + 8, rect.y + 8 + (i * 32), 24, 24, "Test", &checked[i]);
	}
	ui_end_vertical_scroll(&asset_scroll_view);
}

UIScrollView map_matrix_scroll_view;

static void ui_map_matrix_panel(const UIPanelResolved* panel)
{
	if (!panel)	return;

	ui_panel(panel);

	Rect rect = panel->rect;

	ui_begin_scroll(&map_matrix_scroll_view, (Rect) { rect.x + 4, rect.y + 4, rect.width - 8, rect.height - 8 });

	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			if (ui_button(rect.x + 8 + (x * 32), rect.y + 8 + (y * 32), 24, 24, "", 0xFF000000))
			{
					
			}
		}
	}

	ui_end_scroll(&map_matrix_scroll_view);
}

void ui_init_default_layout(void)
{
	static UIPanelLayout panel_asset = { "Assets", PANEL_ASSET_BROWSER, DOCK_LEFT, {0}, true, ui_asset_panel };
	static UIPanelLayout panel_layers = { "Layers", PANEL_MAP_LAYERS, DOCK_LEFT, {0}, true, ui_layers_panel };
	static UIPanelLayout panel_palette = { "Palettes", PANEL_PALETTE_INSPECTOR, DOCK_RIGHT, {0}, true, ui_panel };
	static UIPanelLayout panel_console = { "Console", PANEL_CONSOLE, DOCK_BOTTOM, {0}, true, ui_panel };
	static UIPanelLayout panel_map = { "Map", PANEL_MAP, DOCK_CENTER, {0}, true, ui_map_panel };
	static UIPanelLayout panel_map_matrix = { "Matrix", PANEL_MATRIX, DOCK_RIGHT, {0}, true, ui_map_matrix_panel };

	UIDockNode* left_leaf = make_leaf(&panel_asset);
	UIDockNode* layers_leaf = make_leaf(&panel_layers);
	UIDockNode* map_leaf = make_leaf(&panel_map);
	UIDockNode* center_split = make_split(SPLIT_VERTICAL, 0.15f, layers_leaf, map_leaf);

	UIDockNode* matrix_leaf = make_leaf(&panel_map_matrix);
	UIDockNode* palette_leaf = make_leaf(&panel_palette);
	UIDockNode* right_split = make_split(SPLIT_HORIZONTAL, 0.20f, matrix_leaf, palette_leaf);

	UIDockNode* bottom_leaf = make_leaf(&panel_console);

	UIDockNode* center_vs_bottom = make_split(SPLIT_HORIZONTAL, 0.80f, center_split, bottom_leaf);
	UIDockNode* center_right_split = make_split(SPLIT_VERTICAL, 0.80f, center_vs_bottom, right_split);

	dock_root = make_split(SPLIT_VERTICAL, 0.15f, left_leaf, center_right_split);
}

void ui_resolve_dock_layout(UIDockNode* node, Rect parent)
{
	node->rect = parent;

	if (node->type == NODE_SPLIT)
	{
		Rect a = parent;
		Rect b = parent;

		if (node->split_type == SPLIT_VERTICAL)
		{
			int split_w = (int)(parent.width * node->ratio);
			a.width = split_w;
			b.x += split_w;
			b.width -= split_w;
		}
		else
		{
			int split_h = (int)(parent.height * node->ratio);
			a.height = split_h;
			b.y += split_h;
			b.height -= split_h;
		}

		ui_resolve_dock_layout(node->child_a, a);
		ui_resolve_dock_layout(node->child_b, b);
	}
	else if (node->type == NODE_LEAF)
	{
		for (int i = 0; i < node->panel_count; i++)
		{
			node->panels[i]->rect = parent;
		}
	}
}

void ui_draw_layout_node(UIDockNode* node)
{
	if (!node || !node->rect.width || !node->rect.height) return;

	if (node->type == NODE_SPLIT)
	{
		if (node->split_type == SPLIT_VERTICAL)
		{
			//int split_x = node->rect.x + (int)(node->rect.width * node->ratio);
			//draw_rect(split_x - 2, node->rect.y, 4, node->rect.height, 0xFF444444);
		}
		else
		{
			//int split_y = node->rect.y + (int)(node->rect.height * node->ratio);
			//draw_rect(node->rect.x, split_y - 2, node->rect.width, 4, 0xFF444444);
		}

		ui_draw_layout_node(node->child_a);
		ui_draw_layout_node(node->child_b);
	}
	else if (node->type == NODE_LEAF)
	{
		int tab_height = 24;
		for (int i = 0; i < node->panel_count; ++i)
		{
			UIPanelLayout* panel = node->panels[i];
			Rect tab_rect = {
				.x = node->rect.x + i * 100,
				.y = node->rect.y,
				.width = 100,
				.height = tab_height
			};

			const UISkin* skin = ui_get_skin();

			if (i == node->active_index)
			{
				ui_draw_nineslice(
					tab_rect.x, tab_rect.y, tab_rect.width, tab_rect.height,
					skin->menu_button_pressed.pixels,
					skin->active_palette,
					skin->menu_button_pressed.depth,
					skin->menu_button_pressed.width,
					skin->menu_button_pressed.height,
					8, 8, 8, 8);
			}
			else
			{
				ui_draw_nineslice(
					tab_rect.x, tab_rect.y, tab_rect.width, tab_rect.height,
					skin->menu_button_normal.pixels,
					skin->active_palette,
					skin->menu_button_normal.depth,
					skin->menu_button_normal.width,
					skin->menu_button_normal.height,
					8, 8, 8, 8);
			}

			int label_width = ui_text_width(panel->title);
			int label_height = ui_text_height(panel->title, label_width);

			ui_draw_text_colored(tab_rect.x + (tab_rect.width - label_width) / 2, tab_rect.y + (tab_rect.height - label_height) / 2, panel->title, 0xFF808080);
		}

		UIPanelLayout* active = node->panels[node->active_index];
		Rect content_rect = {
			.x = node->rect.x,
			.y = node->rect.y + tab_height,
			.width = node->rect.width,
			.height = node->rect.height - tab_height
		};

		UIPanelResolved resolved = {
			.type = active->type,
			.rect = content_rect,
			.visible = active->visible
		};

		if (active->draw_func)
			active->draw_func(&resolved);
	}
}

void ui_draw_layout()
{
	ui_draw_layout_node(dock_root);
}
