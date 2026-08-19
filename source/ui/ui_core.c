#include "ui/ui_core.h"
#include "ui/ui.h"

int32_t ui_gen_id(void)
{
    return g_ui.next_id++;
}

void ui_begin_frame(int32_t mouse_x, int32_t mouse_y, bool mouse_down, bool nav_activate)
{
    g_ui.mouse_x = mouse_x;
    g_ui.mouse_y = mouse_y;
    g_ui.mouse_down = mouse_down;
    g_ui.nav_activate = nav_activate;
    g_ui.hot_item = 0;
    g_ui.next_id = 1;
    g_ui.clip_x = 0;
    g_ui.clip_y = 0;
    g_ui.clip_w = FB_WIDTH;
    g_ui.clip_h = FB_HEIGHT;

    if (g_ui.focused_id == 0)
        g_ui.focused_id = 1;
}

void ui_end_frame(void)
{
    if (!g_ui.mouse_down)
    {
        g_ui.active_item = 0;
    }
}