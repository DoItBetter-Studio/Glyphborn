#include "ui_skin.h"
#include "images/ui/glyphborn_windowskin.h"

static const UISkin* g_current_skin;

static const unsigned char* glyphborn_palettes[] = {
	glyphborn_windowskin0_palette_default,
};

static const UISkin g_skins[] = {
	[SKIN_GLYPHBORN] =
	{
		.active_palette = glyphborn_windowskin0_palette_default,
		.palettes = glyphborn_palettes,
		.panel = {
			.pixels = glyphborn_windowskin0_pixels,
			.depth = GLYPHBORN_WINDOWSKIN0_BITDEPTH,
			.width = GLYPHBORN_WINDOWSKIN0_WIDTH,
			.height = GLYPHBORN_WINDOWSKIN0_HEIGHT,
		},
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
