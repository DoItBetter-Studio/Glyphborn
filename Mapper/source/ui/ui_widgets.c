#include "ui/ui_widgets.h"
#include "ui/ui_render.h"
#include "ui/ui_skin.h"

static UIScrollView* g_current_scroll;

#pragma region Basic UI
static void ui_expand_bounds(int x, int y, int width, int height)
{
	if (!g_current_scroll) return;

	int right = x + width;
	int bottom = y + height;

	if (right > g_current_scroll->context_width)
		g_current_scroll->context_width = right;
	if (bottom > g_current_scroll->context_width)
		g_current_scroll->context_height = bottom;
}

void ui_label(int x, int y, const char* text, uint32_t color)
{
	int text_width = ui_text_width(text);

	ui_draw_text_colored(x, y, text, color);
	ui_expand_bounds(x, y, text_width, ui_text_height(text, text_width));
}

bool ui_button(int x, int y, int width, int height, const char* label, uint32_t text_color)
{
	int id = ui_gen_id();

	bool hover, click;
	ui_get_interaction(id, &hover, &click);

	const UISkin* skin = ui_get_skin();

	const unsigned char* color;

	if (click)
		color = skin->button_pressed.pixels;
	else if (hover)
		color = skin->button_hover.pixels;
	else
		color = skin->button_normal.pixels;

	int label_w = ui_text_width(label);
	int label_h = ui_text_height(label, width);

	int text_x = x + (width - label_w) / 2;
	int text_y = y + (height - label_h) / 2;

	ui_draw_nineslice(x, y, width, height, color, skin->active_palette, skin->button_normal.depth, skin->button_normal.width, skin->button_normal.height, 8, 8, 8, 8);
	ui_draw_text_colored(text_x, text_y, label, text_color);

	if (!g_ui.mouse_consumed)
	{
		hover = ui_mouse_inside(x, y, width, height);
		click = ui_mouse_inside(x, y, width, height) && g_ui.mouse_pressed;
		ui_set_interaction(id, hover, click);
	}

	ui_expand_bounds(x, y, width, height);
	return click;
}

bool ui_checkbox(int x, int y, int width, int height, const char* label, bool* value)
{
	int id = ui_gen_id();
	bool hover, click;
	ui_get_interaction(id, &hover, &click);

	if (click) *value = !*value;

	const UISkin* skin = ui_get_skin();

	if (*value && !hover)
	{
		ui_draw_nineslice(x, y, width, height,
			skin->button_checked.pixels,
			skin->active_palette,
			skin->button_checked.depth,
			skin->button_checked.width,
			skin->button_checked.height,
			8, 8, 8, 8);
	}
	else if (*value && hover)
	{
		ui_draw_nineslice(x, y, width, height,
			skin->button_checked_hover.pixels,
			skin->active_palette,
			skin->button_normal.depth,
			skin->button_normal.width,
			skin->button_normal.height,
			8, 8, 8, 8);
	}
	else if (hover)
	{
		ui_draw_nineslice(x, y, width, height,
			skin->button_hover.pixels,
			skin->active_palette,
			skin->button_normal.depth,
			skin->button_normal.width,
			skin->button_normal.height,
			8, 8, 8, 8);
	}
	else
	{
		ui_draw_nineslice(x, y, width, height,
			skin->button_normal.pixels,
			skin->active_palette,
			skin->button_normal.depth,
			skin->button_normal.width,
			skin->button_normal.height,
			8, 8, 8, 8);
	}

	int text_w = ui_text_height(label, width + 64);

	ui_draw_text_colored(x + width + 4, y + (height - text_w) / 2, label, 0xFFFFFFFF);

	if (!g_ui.mouse_consumed)
	{
		hover = ui_mouse_inside(x, y, width, height);
		click = hover && g_ui.mouse_pressed;
		ui_set_interaction(id, hover, click);
	}

	ui_expand_bounds(x, y, width, height);
	return *value;
}
#pragma endregion

#pragma region Scroll Views
#pragma region Scroll
void ui_begin_scroll(UIScrollView* view, Rect rect)
{
	ui_push_clip(rect.x, rect.y, rect.width, rect.height);
	ui_translate(-view->offset_x, -view->offset_y);

	view->context_width = 0;
	view->context_height = 0;
	view->rect = rect;

	g_current_scroll = view;
}

void ui_end_scroll(UIScrollView* view)
{
	g_current_scroll = NULL;

	ui_pop_transform();
	ui_pop_clip();

	if (view->context_height > view->rect.height)
	{
		float ratio = ((float)view->rect.height - 4) / view->context_height;
		int handle_h = (int)(view->rect.height * ratio);
		int handle_y = view->rect.y + 4 + ((int)view->offset_y * ratio);

		if (ui_button(view->rect.x + view->rect.width - 16, handle_y, 12, handle_h, "", 0))
		{

		}
	}

	if (view->context_width > view->rect.width)
	{
		float ratio = (float)view->rect.width / view->context_width;
		int handle_w = (int)view->rect.width * ratio;
		int handle_x = view->rect.x + ((int)view->offset_x * ratio);

		if (ui_button(handle_x, view->rect.y + view->rect.height, handle_w, 12, "", 0))
		{

		}
	}
}
#pragma endregion

#pragma region Vertical Scroll
void ui_begin_vertical_scroll(UIScrollView* view, Rect rect)
{
	ui_begin_scroll(view, rect);
}

void ui_end_vertical_scroll(UIScrollView* view)
{
	g_current_scroll = NULL;

	ui_pop_transform();
	ui_pop_clip();

	if (view->context_height > view->rect.height)
	{
		float ratio = ((float)view->rect.height - 4) / view->context_height;
		int handle_h = (int)(view->rect.height * ratio);
		int handle_y = view->rect.y + 4 + ((int)view->offset_y * ratio);

		if (ui_button(view->rect.x + view->rect.width - 16, handle_y, 12, handle_h, "", 0))
		{

		}
	}
}
#pragma endregion

#pragma region Horizontal Scroll
void ui_begin_horizontal_scroll(UIScrollView* view, Rect rect)
{
	ui_begin_scroll(view, rect);
}

void ui_end_horizontal_scroll(UIScrollView* view)
{
	if (view->context_width > view->rect.width)
	{
		float ratio = (float)view->rect.width / view->context_width;
		int handle_w = (int)view->rect.width * ratio;
		int handle_x = view->rect.x + ((int)view->offset_x * ratio);

		if (ui_button(handle_x, view->rect.y + view->rect.height, handle_w, 12, "", 0))
		{

		}
	}
}
#pragma endregion
#pragma endregion
