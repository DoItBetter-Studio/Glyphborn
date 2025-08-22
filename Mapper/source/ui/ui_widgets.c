#include "ui/ui_widgets.h"
#include "ui/ui_render.h"
#include "ui/ui_skin.h"
#include "input.h"
#include <stddef.h>
#include <stdio.h>

static UIScrollView* g_current_scroll;

#pragma region Basic UI
static void ui_expand_bounds(int x, int y, int width, int height)
{
	if (!g_current_scroll) return;

	int local_x = x - g_current_scroll->rect.x;
	int local_y = y - g_current_scroll->rect.y;

	int right = local_x + width;
	int bottom = local_y + height;

	if (right > g_current_scroll->context_width)
		g_current_scroll->context_width = right;
	if (bottom > g_current_scroll->context_height)
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
		click = ui_mouse_inside(x, y, width, height) && input_get_mouse(MOUSE_LEFT);
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

	if (click && input_get_mouse_up(MOUSE_LEFT)) *value = !*value;

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

	int label_w = ui_text_width(label);
	int label_h = ui_text_height(label, label_w);

	int text_x = x + width + 4;
	int text_y = y + (height - label_h) / 2;

	ui_draw_text_colored(text_x, text_y, label, 0xFFFFFFFF);

	if (!g_ui.mouse_consumed)
	{
		hover = ui_mouse_inside(x, y, width, height);
		click = hover && input_get_mouse(MOUSE_LEFT);
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

	const UISkin* skin = ui_get_skin();

	ui_draw_nineslice(rect.x, rect.y, rect.width, rect.height,
			skin->menu.pixels,
			skin->active_palette,
			skin->menu.depth,
			skin->menu.width,
			skin->menu.height,
			8, 8, 8, 8);
}

void ui_end_scroll(UIScrollView* view)
{
	g_current_scroll = NULL;

	ui_pop_transform();
	ui_pop_clip();

	if (view->context_height > view->rect.height)
	{
		int content_h = view->context_height + 24;
		int view_h = view->rect.height;

		if (view->offset_y < 0) view->offset_y = 0;

		if (view->offset_y < 0) view->offset_y = 0;
		if (view->offset_y > content_h - view_h)
		if (view->offset_y > content_h - view_h)
			view->offset_y = content_h - view_h;

		float ratio = (float)view_h / content_h;
		if (ratio > 1.0f) ratio = 1.0f;

		int handle_h = (int)(ratio * view_h);

		int track_h = view_h - handle_h;
		int handle_y = view->rect.y + (int)((float)view->offset_y / (content_h - view_h) * track_h);

		if (ui_button(view->rect.x + view->rect.width - 20, handle_y + 4, 16, handle_h - 32, "", 0))
		{
			int dy = input_get_mouse_delta().y;

			float scroll_per_px = (float)(content_h - view_h) / track_h;
			view->offset_y += (int)(dy * scroll_per_px);
		}
	}

	if (view->context_width > view->rect.width)
	{
		int content_w = view->context_width + 24;
		int view_w = view->rect.width;

		if (view->offset_x < 0) view->offset_x = 0;
		if (view->offset_x > content_w - view_w)
			view->offset_x = content_w - view_w;

		float ratio = (float)view_w / content_w;
		if (ratio > 1.0f) ratio = 1.0f;

		int handle_w = (int)(ratio * view_w);

		int track_w = view_w - handle_w;
		int handle_x = view->rect.x + (int)((float)view->offset_x / (content_w - view_w) * track_w);

		if (ui_button(handle_x + 4, view->rect.y + view->rect.height - 20, handle_w - 32, 16, "", 0))
		{
			int dx = input_get_mouse_delta().x;

			float scroll_per_px = (float)(content_w - view_w) / track_w;
			view->offset_x += (int)(dx * scroll_per_px);
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
		int content_h = view->context_height + 4;
		int view_h = view->rect.height;

		if (view->offset_y < 0) view->offset_y = 0;
		if (view->offset_y > content_h - view_h)
			view->offset_y = content_h - view_h;

		float ratio = (float)view_h / content_h;
		if (ratio > 1.0f) ratio = 1.0f;

		int handle_h = (int)(ratio * view_h);

		int track_h = view_h - handle_h;
		int handle_y = view->rect.y + (int)((float)view->offset_y / (content_h - view_h) * track_h);

		if (ui_button(view->rect.x + view->rect.width - 20, handle_y + 4, 16, handle_h - 8, "", 0))
		{
			int dy = input_get_mouse_delta().y;

			float scroll_per_px = (float)(content_h - view_h) / track_h;
			view->offset_y += (int)(dy * scroll_per_px);
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
	g_current_scroll = NULL;

	ui_pop_transform();
	ui_pop_clip();

	if (view->context_width > view->rect.width)
	{
		int content_w = view->context_width + 4;
		int view_w = view->rect.width;

		if (view->offset_y < 0) view->offset_y = 0;
		if (view->offset_y > content_w - view_w)
			view->offset_y = content_w - view_w;

		float ratio = (float)view_w / content_w;
		if (ratio > 1.0f) ratio = 1.0f;

		int handle_w = (int)(ratio * view_w);

		int track_w = view_w - handle_w;
		int handle_x = view->rect.y + (int)((float)view->offset_y / (content_w - view_w) * track_w);

		if (ui_button(handle_x + 4, view->rect.y + view->rect.height - 20, handle_w - 8, 16, "", 0))
		{
			int dx = input_get_mouse_delta().x;

			float scroll_per_px = (float)(content_w - view_w) / track_w;
			view->offset_x += (int)(dx * scroll_per_px);
		}
	}
}
#pragma endregion
#pragma endregion
