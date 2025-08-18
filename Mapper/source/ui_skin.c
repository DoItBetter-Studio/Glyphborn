#include "ui_skin.h"
#include "images/ui/glyphborn_mapper_windowskin.h"

static UISkin* g_current_skin;

static const unsigned char* glyphborn_palettes[] = {
	[PALETTE_DEFAULT]	= glyphborn_mapper_panel_palette_default,
	[PALETTE_TEST]		= glyphborn_mapper_test_palette
};

static UISkin g_skins[] = {
	[SKIN_GLYPHBORN] =
	{
		.active_palette = glyphborn_mapper_test_palette,
		.palettes = glyphborn_palettes,
		.menu_bar = {
			.pixels = glyphborn_mapper_menu_bar_pixels,
			.depth = GLYPHBORN_MAPPER_MENU_BAR_BITDEPTH,
			.width = GLYPHBORN_MAPPER_MENU_BAR_WIDTH,
			.height = GLYPHBORN_MAPPER_MENU_BAR_HEIGHT,
		},
		.button_normal = {
			.pixels = glyphborn_mapper_button_normal_pixels,
			.depth = GLYPHBORN_MAPPER_BUTTON_NORMAL_BITDEPTH,
			.width = GLYPHBORN_MAPPER_BUTTON_NORMAL_WIDTH,
			.height = GLYPHBORN_MAPPER_BUTTON_NORMAL_HEIGHT,
		},
		.button_hover = {
			.pixels = glyphborn_mapper_button_hover_pixels,
			.depth = GLYPHBORN_MAPPER_BUTTON_HOVER_BITDEPTH,
			.width = GLYPHBORN_MAPPER_BUTTON_HOVER_WIDTH,
			.height = GLYPHBORN_MAPPER_BUTTON_HOVER_HEIGHT,
		},
		.button_pressed = {
			.pixels = glyphborn_mapper_button_pressed_pixels,
			.depth = GLYPHBORN_MAPPER_BUTTON_PRESSED_BITDEPTH,
			.width = GLYPHBORN_MAPPER_BUTTON_PRESSED_WIDTH,
			.height = GLYPHBORN_MAPPER_BUTTON_PRESSED_HEIGHT,
		},
		.panel = {
			.pixels = glyphborn_mapper_panel_pixels,
			.depth = GLYPHBORN_MAPPER_PANEL_BITDEPTH,
			.width = GLYPHBORN_MAPPER_PANEL_WIDTH,
			.height = GLYPHBORN_MAPPER_PANEL_HEIGHT,
		}
	}
};

const UISkin* ui_get_skin()
{
	return g_current_skin;
}

void ui_set_skin(Skin skin)
{
	if (skin >= 0 && skin < SKIN_COUNT) {
		g_current_skin = &g_skins[skin];
	}
}

void ui_set_palette(Palette palette)
{
	if (!g_current_skin)
	{
		ui_set_skin(SKIN_GLYPHBORN);
	}

	if (palette >= 0 && palette < PALETTE_COUNT)
	{
		g_current_skin->active_palette = g_current_skin->palettes[palette];
	}
}
