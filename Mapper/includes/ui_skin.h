#ifndef UI_SKIN_H
#define UI_SKIN_H

#include <stdint.h>

typedef enum
{
	SKIN_GLYPHBORN,
	SKIN_COUNT,
} Skin;

typedef struct Element
{
	const unsigned char* pixels;
	int width;
	int height;
	int depth;
} Element;

typedef enum
{
	PALETTE_DEFAULT,
	PALETTE_TEST,
	PALETTE_COUNT
} Palette;

typedef struct UISkin
{
	const unsigned char* active_palette;
	const unsigned char* const* palettes;
	Element menu_bar;
	Element button_normal;
	Element button_hover;
	Element button_pressed;
	Element panel;
} UISkin;

const UISkin* ui_get_skin();
void ui_set_skin(Skin skin);
void ui_set_palette(Palette palette);
#endif // !UI_SKIN_H
