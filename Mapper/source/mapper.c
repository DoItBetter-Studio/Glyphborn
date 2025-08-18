#include "mapper.h"
#include "input.h"
#include "ui.h"

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
}

void mapper_update(float delta_time)
{
	input_update();
}

void mapper_render()
{
}

bool showUI = false;
UIPanelResolved panels[MAX_PANELS];

void mapper_render_ui()
{
	ui_begin_frame(input_get_mouse_x(), input_get_mouse_y(), input_get_mouse_left(), true);

	resolve_layout(&mapper_layout, fb_width, fb_height, panels, MAX_PANELS);

	for (int i = 0; i < MAX_PANELS; ++i)
	{
		if (panels[i].visible)
			ui_panel(&panels[i]);
	}

	if (ui_button(10, 40, 64, 32, "Default", 0xFFFFFFFF))
	{
		ui_set_palette(PALETTE_DEFAULT);
	}

	if (ui_button(10, 75, 64, 32, "Test", 0xFFFFFFFF))
	{
		ui_set_palette(PALETTE_TEST);
	}

	ui_end_frame();
}

void mapper_shutdown()
{

}
