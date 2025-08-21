#include "ui/ui_core.h"
#include "ui/ui_render.h"
#include "input.h"

UIContext g_ui = { 0 };
static UIState g_ui_state;

void ui_begin_frame(int mouse_x, int mouse_y)
{
	g_ui.mouse_x = mouse_x;
	g_ui.mouse_y = mouse_y;
	g_ui.mouse_down = input_get_mouse(MOUSE_LEFT);
	g_ui.mouse_pressed = input_get_mouse_down(MOUSE_LEFT);
	g_ui.mouse_released = input_get_mouse_up(MOUSE_LEFT);
	g_ui.next_id = 1;
	g_ui.current_layer = !g_ui.current_layer ? 0 : g_ui.current_layer;

	g_ui.mouse_consumed = false;
}

void ui_end_frame(void)
{
	for (int i = MAX_UI_ELEMENTS - 1; i >= 0; --i)
	{
		if (g_ui_state.click[i] && !g_ui.mouse_consumed)
		{
			g_ui.mouse_consumed = true;
		}
		else
		{
			g_ui_state.click[i] = false;
		}

		if (g_ui_state.hover[i] && !g_ui.mouse_consumed)
		{
			g_ui.mouse_consumed = true;
		}
		else
		{
			g_ui_state.hover[i] = false;
		}
	}
}

bool ui_mouse_inside(int x, int y, int width, int height)
{
	return g_ui.mouse_x >= x && g_ui.mouse_x < x + width &&
		g_ui.mouse_y >= y && g_ui.mouse_y < y + height;
}

int ui_gen_id(void)
{
	return g_ui.next_id++;
}

void ui_set_interaction(int id, bool hover, bool click)
{
	g_ui_state.click[id] = click;
	g_ui_state.hover[id] = hover;
}

void ui_get_interaction(int id, bool* hover, bool* click)
{
	*hover = g_ui_state.hover[id];
	*click = g_ui_state.click[id];
}

void ui_push_clip(int x, int y, int width, int height)
{
	if (g_ui.clip_top + 1 >= UI_CLIP_STACK_MAX) return;

	g_ui.clip_stack[g_ui.clip_top++] = g_ui.current_clip;

	int nx = (x > g_ui.current_clip.x) ? x : g_ui.current_clip.x;
	int ny = (y > g_ui.current_clip.y) ? y : g_ui.current_clip.y;

	int nw = ((x + width < g_ui.current_clip.x + g_ui.current_clip.width) ? x + width : g_ui.current_clip.x + g_ui.current_clip.width) - nx;
	int nh = ((y + height < g_ui.current_clip.y + g_ui.current_clip.height) ? x + height : g_ui.current_clip.y + g_ui.current_clip.height) - ny;

	if (nw < 0) nw = 0;
	if (nh < 0) nh = 0;

	g_ui.current_clip.x = nx;
	g_ui.current_clip.y = ny;
	g_ui.current_clip.width = nw;
	g_ui.current_clip.height = nh;

	ui_set_clip(nx, ny, nw, nh);
}

void ui_pop_clip(void)
{
	if (g_ui.clip_top <= 0) return;
	g_ui.current_clip = g_ui.clip_stack[--g_ui.clip_top];
	ui_set_clip(g_ui.current_clip.x, g_ui.current_clip.y, g_ui.current_clip.width, g_ui.current_clip.height);
}

void ui_translate(int dx, int dy)
{
	if (g_ui.transform_top + 1 >= UI_TRANSFORM_STACK_MAX) return;
	g_ui.transform_stack[g_ui.transform_top++] = g_ui.current_transform;

	g_ui.current_transform.tx += dx;
	g_ui.current_transform.ty += dy;

	ui_set_transform(g_ui.current_transform.tx, g_ui.current_transform.ty);
}

void ui_pop_transform(void)
{
	if (g_ui.transform_top <= 0) return;
	g_ui.current_transform = g_ui.transform_stack[--g_ui.transform_top];
	ui_set_transform(g_ui.current_transform.tx, g_ui.current_transform.ty);
}
