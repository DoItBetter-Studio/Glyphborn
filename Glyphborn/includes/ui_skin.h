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
	const char* pixels;
	int width;
	int height;
	int depth;
} Element;

typedef struct UISkin
{
	const char* active_palette;
	const uint8_t* const* palettes;
	Element panel;
} UISkin;

const UISkin* ui_get_skin();
void ui_set_skin(Skin skin);
#endif // !UI_SKIN_H
