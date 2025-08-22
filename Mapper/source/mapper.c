#include "mapper.h"
#include "input.h"
#include "render.h"
#include "ui/ui_skin.h"
#include "ui/ui_core.h"
#include "ui/ui_render.h"
#include "ui/ui_layout.h"
#include "ui/ui_menu.h"
#include <stdio.h>
#include <stdlib.h>

static void draw_test_pattern_ui(void)
{
	for (int y = 200; y < fb_height - 16; ++y)
	{
		for (int x = 16; x < fb_width - 16; ++x)
		{
			bool is_light = ((x / 16) + (y / 16)) % 2 == 0;
			uint32_t color = is_light ? 0x88FFFFFF : 0x88000000; // semi-transparent white/black
			framebuffer_ui[y * fb_width + x] = color;
		}
	}
}

void mapper_init()
{
	input_init();
	ui_set_skin(SKIN_GLYPHBORN);

	ui_init_default_layout();
	ui_resolve_dock_layout(dock_root, (Rect) { 0, MENU_BAR_HEIGHT, fb_width, fb_height - MENU_BAR_HEIGHT });
}

void mapper_update(float delta_time)
{
	(void)delta_time;
	
	input_update();
	ui_resolve_dock_layout(dock_root, (Rect) { 0, MENU_BAR_HEIGHT, fb_width, fb_height - MENU_BAR_HEIGHT });
}

void mapper_render() {}

bool showUI = false;
UIPanelResolved panels[MAX_PANELS];

bool action_new() { draw_test_pattern_ui(); return true; }
bool action_open() { return true; }

UIMenuItem file_items[] =
{
	{ "New", "ctrl + n", action_new },
	{ "Open", "ctrl + o", action_open }
};

bool action_default_palette() { ui_set_palette(PALETTE_DEFAULT); return true; }
bool action_dark_palette() { ui_set_palette(PALETTE_DARK); return true; }

UIMenuItem theme_items[] =
{
	{ "Default", NULL, action_default_palette },
	{ "Dark", NULL, action_dark_palette }
};

UIMenu menus[] =
{
	{ "File", file_items, 2, false },
	{ "Themes", theme_items, 2, false }
};

void mapper_render_ui()
{
	ui_begin_frame();

	ui_draw_layout();
	ui_menu_bar(menus, 2);

	ui_end_frame();
}

void mapper_shutdown()
{
	if (framebuffer_ui) free(framebuffer_ui);
	if (framebuffer_mapview) free(framebuffer_mapview);
	if (depthbuffer_mapview) free(depthbuffer_mapview);
}
