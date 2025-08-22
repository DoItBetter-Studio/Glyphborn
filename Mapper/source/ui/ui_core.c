#include "ui/ui_core.h"
#include "ui/ui_render.h"
#include "input.h"
#include "render.h"

UIContext g_ui = { 0 };
static UIState g_ui_state;
Rect g_clip = { 0, 0, 0, 0 };
int g_tx = 0;
int g_ty = 0;

void ui_begin_frame()
{
	Vec2 coord = input_get_mouse_position();

	g_ui.mouse_x = coord.x;
	g_ui.mouse_y = coord.y;
	g_ui.mouse_down = input_get_mouse(MOUSE_LEFT);
	g_ui.mouse_pressed = input_get_mouse_down(MOUSE_LEFT);
	g_ui.mouse_released = input_get_mouse_up(MOUSE_LEFT);
	g_ui.next_id = 1;
	g_ui.current_layer = !g_ui.current_layer ? 0 : g_ui.current_layer;

	g_ui.mouse_consumed = false;

	g_ui.clip_top = 0;
	g_ui.transform_top = 0;

	g_ui.current_clip = (Rect){ 0, 0, fb_width, fb_height };
	ui_set_clip(0, 0, fb_width, fb_height);

	g_ui.current_transform = (UITransform){ 0,0 };
	ui_set_transform(0, 0);
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
	return g_ui.mouse_x >= x + g_tx && g_ui.mouse_x < x + width + g_tx &&
		g_ui.mouse_y >= y + g_ty && g_ui.mouse_y < y + height + g_ty;
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

    int clip_x1 = g_ui.current_clip.x;
    int clip_y1 = g_ui.current_clip.y;
    int clip_x2 = g_ui.current_clip.x + g_ui.current_clip.width;
    int clip_y2 = g_ui.current_clip.y + g_ui.current_clip.height;

    int new_x1 = x;
    int new_y1 = y;
    int new_x2 = x + width;
    int new_y2 = y + height;

    // intersection
    int nx1 = (new_x1 > clip_x1) ? new_x1 : clip_x1;
    int ny1 = (new_y1 > clip_y1) ? new_y1 : clip_y1;
    int nx2 = (new_x2 < clip_x2) ? new_x2 : clip_x2;
    int ny2 = (new_y2 < clip_y2) ? new_y2 : clip_y2;

    int nw = nx2 - nx1;
    int nh = ny2 - ny1;

    if (nw < 0) nw = 0;
    if (nh < 0) nh = 0;

    g_ui.current_clip = (Rect){ nx1, ny1, nw, nh };
    ui_set_clip(nx1, ny1, nw, nh);
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
