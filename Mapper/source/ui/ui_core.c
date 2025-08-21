#include "ui/ui_core.h"
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
