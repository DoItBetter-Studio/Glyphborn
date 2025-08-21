#include "ui/ui_menu.h"
#include "ui/ui_widgets.h"
#include "ui/ui_render.h"
#include "ui/ui_skin.h"
#include "ui/ui_core.h"
#include "render.h"
#include "input.h"

#define MENU_ITEM_HEIGHT	24
#define SUB_MENU_WIDTH		196
#define SHORTCUT_WIDTH		64

static UIMenu* g_active_menus = NULL;
static int g_active_menu_count = 0;

static void ui_menu_draw_item(UIMenuItem* item, int x, int y, int width, int height)
{
	int id = ui_gen_id();

	const UISkin* skin = ui_get_skin();

	bool is_hovered = false;
	bool clicked = false;
	ui_get_interaction(id, &is_hovered, &clicked);

	if (clicked)
	{
		if (item->action) item->action();
		ui_draw_nineslice(x, y, width, height,
			skin->menu_button_pressed.pixels,
			skin->active_palette,
			skin->menu_button_pressed.depth,
			skin->menu_button_pressed.width,
			skin->menu_button_pressed.height,
			8, 8, 8, 8);
	}
	else if (is_hovered)
	{
		ui_draw_nineslice(x, y, width, height,
			skin->menu_button_hover.pixels,
			skin->active_palette,
			skin->menu_button_hover.depth,
			skin->menu_button_hover.width,
			skin->menu_button_hover.height,
			8, 8, 8, 8);
	}
	else
	{
		ui_draw_nineslice(x, y, width, height,
			skin->menu_button_normal.pixels,
			skin->active_palette,
			skin->menu_button_normal.depth,
			skin->menu_button_normal.width,
			skin->menu_button_normal.height,
			8, 8, 8, 8);
	}

	if (item->shortcut)
	{
		int shortcut_x = x + (SHORTCUT_WIDTH - ui_text_width(item->shortcut)) / 2;
		int shortcut_y = y + (height - ui_text_height(item->shortcut, SHORTCUT_WIDTH)) / 2;
		ui_draw_text_colored(shortcut_x, shortcut_y, item->shortcut, 0xFF555555);
	}

	int text_x = x + item->shortcut ? SHORTCUT_WIDTH + 4 : 0;
	int label_width = width - SHORTCUT_WIDTH - 4;
	int text_y = y + (height - ui_text_height(item->label, label_width)) / 2;
	ui_draw_text_colored(text_x, text_y, item->label, 0xFF000000);

	if (!g_ui.mouse_consumed)
	{
		bool hover = ui_mouse_inside(x, y, width, height);
		bool click = ui_mouse_inside(x, y, width, height) && g_ui.mouse_pressed;
		ui_set_interaction(id, hover, click);
	}
}

void ui_menu_bar(UIMenu* menus, int menu_count)
{
	const UISkin* skin = ui_get_skin();

	ui_draw_nineslice(0, 0, fb_width, MENU_BAR_HEIGHT,
		skin->menu.pixels,
		skin->active_palette,
		skin->menu.depth,
		skin->menu.width,
		skin->menu.height,
		8, 8, 8, 8);

	int cursor_x = 4;
	int cursor_y = 4;

	for (int i = 0; i < menu_count; i++)
	{
		int id = ui_gen_id();

		UIMenu* menu = &menus[i];
		int width = ui_text_width(menu->label) + 16;

		const UISkin* skin = ui_get_skin();
		
		bool is_hovered = false;
		bool clicked = false;
		ui_get_interaction(id, &is_hovered, &clicked);


		if (clicked)
		{
			bool was_open = menu->open;

			for (int j = 0; j < menu_count; j++)
				menus[j].open = false;

			menu->open = !was_open;

			for (int j = 0; j < g_active_menu_count; ++j)
				g_active_menus[i].open = false;

			ui_draw_nineslice(cursor_x, cursor_y, width, MENU_BAR_HEIGHT - 8,
				skin->menu_button_pressed.pixels,
				skin->active_palette,
				skin->menu_button_pressed.depth,
				skin->menu_button_pressed.width,
				skin->menu_button_pressed.height,
				8, 8, 8, 8);
		}
		else if (is_hovered)
		{
			ui_draw_nineslice(cursor_x, cursor_y, width, MENU_BAR_HEIGHT - 8,
				skin->menu_button_hover.pixels,
				skin->active_palette,
				skin->menu_button_hover.depth,
				skin->menu_button_hover.width,
				skin->menu_button_hover.height,
				8, 8, 8, 8);
		}
		else
		{
			ui_draw_nineslice(cursor_x, cursor_y, width, MENU_BAR_HEIGHT - 8,
				skin->menu_button_normal.pixels,
				skin->active_palette,
				skin->menu_button_normal.depth,
				skin->menu_button_normal.width,
				skin->menu_button_normal.height,
				8, 8, 8, 8);
		}

		if (menu->open)
		{
			int panel_id = ui_gen_id();

			bool is_hovered = false;
			bool clicked = false;
			ui_get_interaction(panel_id, &is_hovered, &clicked);

			int panel_x = cursor_x - 4;
			int panel_y = MENU_BAR_HEIGHT;
			int panel_w = SUB_MENU_WIDTH;
			int panel_h = MENU_ITEM_HEIGHT * menu->item_count + 8;

			ui_draw_nineslice(panel_x, panel_y, panel_w, panel_h,
				skin->panel.pixels,
				skin->active_palette,
				skin->panel.depth,
				skin->panel.width,
				skin->panel.height,
				4, 4, 4, 4);

			for (int j = 0; j < menu->item_count; j++)
			{
				ui_menu_draw_item(&menu->items[j], panel_x + 4, panel_y + j * MENU_ITEM_HEIGHT + 4, SUB_MENU_WIDTH - 8, MENU_ITEM_HEIGHT);
			}

			if (!g_ui.mouse_consumed)
			{
				bool hover = ui_mouse_inside(panel_x, panel_y, panel_w, panel_h);
				bool click = ui_mouse_inside(panel_x, panel_y, panel_w, panel_h) && g_ui.mouse_pressed;
				ui_set_interaction(panel_id, hover, click);
			}
		}
		
		int text_width = ui_text_width(menu->label);
		int text_height = ui_text_height(menu->label, width);
		int text_x = cursor_x + (width - text_width) / 2;
		int text_y = (MENU_BAR_HEIGHT - text_height) / 2;
		ui_draw_text_colored(text_x, text_y, menu->label, 0xFF000000);

		if (!g_ui.mouse_consumed)
		{
			bool hover = ui_mouse_inside(cursor_x, cursor_y, width, MENU_BAR_HEIGHT - 8);
			bool click = ui_mouse_inside(cursor_x, cursor_y, width, MENU_BAR_HEIGHT - 8) && g_ui.mouse_pressed;
			ui_set_interaction(id, hover, click);
		}

		cursor_x += width + 4;
	}
}