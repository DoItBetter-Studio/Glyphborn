#include "ui/ui_widgets.h"
#include "ui/ui_core.h"
#include "ui/ui_render.h"
#include "ui/ui_skin.h"
#include "render/render.h"

bool ui_button(int32_t x, int32_t y, int32_t width, int32_t height, const char* label, uint32_t text_color)
{
    int32_t id = ui_gen_id();
    bool inside = (g_ui.mouse_x >= x && g_ui.mouse_x < x + width &&
                   g_ui.mouse_y >= y && g_ui.mouse_y < y + height);

    if (inside) g_ui.hot_item = id;

    if (inside && g_ui.mouse_down && g_ui.active_item == 0)
    {
        g_ui.active_item = id;
        g_ui.nav_mode = false;
    }

    bool focused = (g_ui.nav_mode && g_ui.focused_id == id);
    if ((inside && g_ui.mouse_down && g_ui.active_item == 0) || (focused && g_ui.nav_activate))
    {
        g_ui.active_item = id;
    }

    bool clicked = false;
    if ((!g_ui.mouse_down && g_ui.active_item == id && inside) || (focused && g_ui.nav_activate))
    {
        clicked = true;
    }

    int32_t label_w = ui_text_width(label);
    int32_t label_h = ui_text_height(label, width - 32);

    int32_t text_x = x + (width - label_w) / 2;
    int32_t text_y = y + (height - label_h) / 2;

    const UISkin* skin = ui_get_skin();
    ui_draw_nineslice(x, y, width, height, skin->panel.pixels, skin->panel.width, skin->panel.height, 16, 16, 16, 16);
    ui_draw_text_colored(text_x, text_y, label, text_color);

    return clicked;
}

void ui_label(int32_t x, int32_t y, const char* text)
{
    ui_draw_text_colored(x, y, text, UI_COLOR_BLACK);
}

void ui_label_colored(int32_t x, int32_t y, const char* text, uint32_t color)
{
    ui_draw_text_colored(x, y, text, color);
}

void ui_bar(int32_t x, int32_t y, int32_t width, int32_t height, float value)
{
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    const UISkin* skin = ui_get_skin();
    if (skin->bar_track.pixels)
        ui_draw_nineslice(x, y, width, height, skin->bar_track.pixels, skin->bar_track.width, skin->bar_track.height, 4, 4, 4, 4);
    
    int32_t fill_width = (int32_t)(width * value);
    if (fill_width > 0 && skin->bar_fill.pixels)
        ui_draw_nineslice(x, y, fill_width, height, skin->bar_fill.pixels, skin->bar_fill.width, skin->bar_fill.height, 4, 4, 4, 4);
}

void ui_dialogue_box(int32_t x, int32_t y, int32_t width, int32_t height, const char* text, const uint32_t* portrait)
{
    const UISkin* skin = ui_get_skin();
    if (skin->dialogue_box.pixels)
        ui_draw_nineslice(x, y, width, height, skin->dialogue_box.pixels, skin->dialogue_box.width, skin->dialogue_box.height, 16, 16, 16, 16);

    int32_t text_x = x + 8;
    int32_t text_y = y + 8;
    int32_t text_w = width - 16;

    if (portrait && skin->portrait_frame.pixels)
    {
        int32_t portrait_outer = height - 16;
        int32_t border = 8;
        int32_t portrait_inner = portrait_outer - border * 2;

        ui_draw_nineslice(x + 8, y + 8, portrait_outer, portrait_outer, skin->portrait_frame.pixels, skin->portrait_frame.width, skin->portrait_frame.height, border, border, border, border);

        int32_t dst_x = x + 8 + border;
        int32_t dst_y = y + 8 + border;

        for (int32_t py = 0; py < portrait_inner && (dst_y + py) < FB_HEIGHT; py++)
        {
            for (int32_t px = 0; px < portrait_inner && (dst_x + px) < FB_WIDTH; px++)
            {
                uint32_t pixel = portrait[py * portrait_inner + px];
                if ((pixel >> 24) == 0) continue;
                framebuffer_ui[(dst_y + py) * FB_WIDTH + (dst_x + px)] = pixel;
            }
        }
        text_x = x + portrait_outer + 16;
        text_w = width - portrait_outer - 24;
    }

    if (text)
        ui_draw_text_wrapped(text_x, text_y, text_w, text, 0xFF000000);
}

float ui_scrollbar_vertical(int32_t x, int32_t y, int32_t width, int32_t height, float value)
{
    int32_t id = ui_gen_id();
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    const UISkin* skin = ui_get_skin();
    if (skin->scrollbar_track.pixels)
        ui_draw_nineslice(x, y, width, height, skin->scrollbar_track.pixels, skin->scrollbar_track.width, skin->scrollbar_track.height, 4, 4, 4, 4);

    int32_t thumb_h = height / 4;
    if (thumb_h < 16) thumb_h = 16;

    int32_t thumb_travel = height - thumb_h;
    int32_t thumb_y = y + (int32_t)(thumb_travel * value);

    bool inside_thumb = (g_ui.mouse_x >= x && g_ui.mouse_x < x + width && g_ui.mouse_y >= thumb_y && g_ui.mouse_y < thumb_y + thumb_h);
    bool inside_track = (g_ui.mouse_x >= x && g_ui.mouse_x < x + width && g_ui.mouse_y >= y && g_ui.mouse_y < y + height);

    if (inside_thumb) g_ui.hot_item = id;
    if (inside_thumb && g_ui.mouse_down && g_ui.active_item == 0) g_ui.active_item = id;

    if (g_ui.active_item == id && g_ui.mouse_down)
    {
        int32_t relative_y = g_ui.mouse_y - y - thumb_h / 2;
        if (relative_y < 0) relative_y = 0;
        if (relative_y > thumb_travel) relative_y = thumb_travel;
        value = (thumb_travel > 0) ? (float)relative_y / (float)thumb_travel : 0.0f;
        thumb_y = y + (int32_t)(thumb_travel * value);
    }

    if (inside_track && !inside_thumb && g_ui.mouse_down && g_ui.active_item == 0)
    {
        int32_t relative_y = g_ui.mouse_y - y - thumb_h / 2;
        if (relative_y < 0) relative_y = 0;
        if (relative_y > thumb_travel) relative_y = thumb_travel;
        value = (thumb_travel > 0) ? (float)relative_y / (float)thumb_travel : 0.0f;
        thumb_y = y + (int32_t)(thumb_travel * value);
        g_ui.active_item = id;
    }

    const Element* thumb_elem = &skin->scrollbar_thumb;
    if (g_ui.active_item == id && skin->scrollbar_thumb_hot.pixels) thumb_elem = &skin->scrollbar_thumb_hot;
    else if (g_ui.hot_item == id && skin->scrollbar_thumb_hot.pixels) thumb_elem = &skin->scrollbar_thumb_hot;

    ui_set_clip(x, y, width, height);
    if (thumb_elem->pixels)
        ui_draw_nineslice(x, thumb_y, width, thumb_h, thumb_elem->pixels, thumb_elem->width, thumb_elem->height, 4, 4, 4, 4);
    ui_reset_clip();

    return value;
}

float ui_scrollbar_horizontal(int32_t x, int32_t y, int32_t width, int32_t height, float value)
{
    int32_t id = ui_gen_id();
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;

    const UISkin* skin = ui_get_skin();
    if (skin->scrollbar_track.pixels)
        ui_draw_nineslice(x, y, width, height, skin->scrollbar_track.pixels, skin->scrollbar_track.width, skin->scrollbar_track.height, 4, 4, 4, 4);

    int32_t thumb_w = width / 4;
    if (thumb_w < 16) thumb_w = 16;

    int32_t thumb_travel = width - thumb_w;
    int32_t thumb_x = x + (int32_t)(thumb_travel * value);

    bool inside_thumb = (g_ui.mouse_x >= thumb_x && g_ui.mouse_x < thumb_x + thumb_w && g_ui.mouse_y >= y && g_ui.mouse_y < y + height);
    bool inside_track = (g_ui.mouse_x >= x && g_ui.mouse_x < x + width && g_ui.mouse_y >= y && g_ui.mouse_y < y + height);

    if (inside_thumb) g_ui.hot_item = id;
    if (inside_thumb && g_ui.mouse_down && g_ui.active_item == 0) g_ui.active_item = id;

    if (g_ui.active_item == id && g_ui.mouse_down)
    {
        int32_t relative_x = g_ui.mouse_x - x - thumb_w / 2;
        if (relative_x < 0) relative_x = 0;
        if (relative_x > thumb_travel) relative_x = thumb_travel;
        value = (thumb_travel > 0) ? (float)relative_x / (float)thumb_travel : 0.0f;
        thumb_x = x + (int32_t)(thumb_travel * value);
    }

    if (inside_track && !inside_thumb && g_ui.mouse_down && g_ui.active_item == 0)
    {
        int32_t relative_x = g_ui.mouse_x - x - thumb_w / 2;
        if (relative_x < 0) relative_x = 0;
        if (relative_x > thumb_travel) relative_x = thumb_travel;
        value = (thumb_travel > 0) ? (float)relative_x / (float)thumb_travel : 0.0f;
        thumb_x = x + (int32_t)(thumb_travel * value);
        g_ui.active_item = id;
    }

    const Element* thumb_elem = &skin->scrollbar_thumb;
    if (g_ui.active_item == id && skin->scrollbar_thumb_hot.pixels) thumb_elem = &skin->scrollbar_thumb_hot;
    else if (g_ui.hot_item == id && skin->scrollbar_thumb_hot.pixels) thumb_elem = &skin->scrollbar_thumb_hot;

    ui_set_clip(x, y, width, height);
    if (thumb_elem->pixels)
        ui_draw_nineslice(thumb_x, y, thumb_w, height, thumb_elem->pixels, thumb_elem->width, thumb_elem->height, 4, 4, 4, 4);
    ui_reset_clip();

    return value;
}

UIScrollValue ui_scrollbar_both(int32_t x, int32_t y, int32_t width, int32_t height, UIScrollValue value)
{
    value.y = ui_scrollbar_vertical(x + width - 12, y, 12, height - 12, value.y);
    value.x = ui_scrollbar_horizontal(x, y + height - 12, width - 12, 12, value.x);

    const UISkin* skin = ui_get_skin();
    if (skin->scrollbar_track.pixels)
        ui_draw_nineslice(x + width - 12, y + height - 12, 12, 12, skin->scrollbar_track.pixels, skin->scrollbar_track.width, skin->scrollbar_track.height, 4, 4, 4, 4);

    return value;
}

bool ui_item_slot(int32_t x, int32_t y, int32_t size, const uint32_t* item_pixels, int32_t item_width, int32_t item_height)
{
    int32_t id = ui_gen_id();
    bool inside = (g_ui.mouse_x >= x && g_ui.mouse_x < x + size && g_ui.mouse_y >= y && g_ui.mouse_y < y + size);

    if (inside) g_ui.hot_item = id;
    if (inside && g_ui.mouse_down && g_ui.active_item == 0)
    {
        g_ui.active_item = id;
        g_ui.nav_mode = false;
    }

    bool focused = (g_ui.nav_mode && g_ui.focused_id == id);
    if ((inside && g_ui.mouse_down && g_ui.active_item == 0) || (focused && g_ui.nav_activate))
        g_ui.active_item = id;

    bool clicked = false;
    if ((!g_ui.mouse_down && g_ui.active_item == id && inside) || (focused && g_ui.nav_activate))
        clicked = true;

    const UISkin* skin = ui_get_skin();
    const Element* slot_elem = &skin->item_slot;
    if (g_ui.active_item == id && skin->item_slot_active.pixels) slot_elem = &skin->item_slot_active;
    else if ((g_ui.hot_item == id || focused) && skin->item_slot_hot.pixels) slot_elem = &skin->item_slot_hot;

    if (slot_elem->pixels)
        ui_draw_nineslice(x, y, size, size, slot_elem->pixels, slot_elem->width, slot_elem->height, 8, 8, 8, 8);

    if (item_pixels)
    {
        int32_t padding = 4;
        int32_t inner = size - padding * 2;
        int32_t dst_x = x + padding;
        int32_t dst_y = y + padding;

        int32_t clip_x2 = g_ui.clip_x + g_ui.clip_w;
        int32_t clip_y2 = g_ui.clip_y + g_ui.clip_h;

        for (int32_t py = 0; py < inner && (dst_y + py) < FB_HEIGHT; py++)
        {
            if ((dst_y + py) < g_ui.clip_y || (dst_y + py) >= clip_y2) continue;

            for (int32_t px = 0; px < inner && (dst_x + px) < FB_WIDTH; px++)
            {
                if ((dst_x + px) < g_ui.clip_x || (dst_x + px) >= clip_x2) continue;

                int32_t src_x = px * item_width / inner;
                int32_t src_y = py * item_height / inner;

                uint32_t pixel = item_pixels[src_y * item_width + src_x];
                if ((pixel >> 24) == 0) continue;

                framebuffer_ui[(dst_y + py) * FB_WIDTH + (dst_x + px)] = pixel;
            }
        }
    }
    return clicked;
}

bool ui_checkbox(int32_t x, int32_t y, int32_t size, bool checked)
{
    int32_t id = ui_gen_id();
    bool inside = (g_ui.mouse_x >= x && g_ui.mouse_x < x + size && g_ui.mouse_y >= y && g_ui.mouse_y < y + size);

    if (inside) g_ui.hot_item = id;
    if (inside && g_ui.mouse_down && g_ui.active_item == 0)
    {
        g_ui.active_item = id;
        g_ui.nav_mode = false;
    }

    bool focused = (g_ui.nav_mode && g_ui.focused_id == id);
    if ((inside && g_ui.mouse_down && g_ui.active_item == 0) || (focused && g_ui.nav_activate))
        g_ui.active_item = id;

    if ((!g_ui.mouse_down && g_ui.active_item == id && inside) || (focused && g_ui.nav_activate))
        checked = !checked;

    const UISkin* skin = ui_get_skin();
    const Element* elem = checked ? &skin->checkbox_checked : &skin->checkbox;

    if (!elem->pixels)
        elem = checked ? &skin->checkbox : &skin->checkbox_checked;

    if (elem->pixels)
        ui_draw_nineslice(x, y, size, size, elem->pixels, elem->width, elem->height, 4, 4, 4, 4);

    return checked;
}

void ui_row_highlight(int32_t x, int32_t y, int32_t width, int32_t height)
{
    const UISkin* skin = ui_get_skin();
    if (skin->row_highlight.pixels)
        ui_draw_nineslice(x, y, width, height, skin->row_highlight.pixels, skin->row_highlight.width, skin->row_highlight.height, 4, 4, 4, 4);
}

void ui_panel(int32_t x, int32_t y, int32_t width, int32_t height)
{
    const UISkin* skin = ui_get_skin();
    if (skin->panel.pixels)
        ui_draw_nineslice(x, y, width, height, skin->panel.pixels, skin->panel.width, skin->panel.height, 16, 16, 16, 16);
}

void ui_window(int32_t x, int32_t y, int32_t width, int32_t height, const char* title)
{
    const UISkin* skin = ui_get_skin();
    if (skin->window.pixels)
        ui_draw_nineslice(x, y, width, height, skin->window.pixels, skin->window.width, skin->window.height, 16, 16, 16, 16);

    if (title)
    {
        int32_t title_w = ui_text_width(title);
        int32_t title_x = x + (width - title_w) / 2;
        ui_draw_text_colored(title_x, y + 4, title, UI_COLOR_BLACK);
    }
}

void ui_tooltip(int32_t x, int32_t y, int32_t width, int32_t height, const char* text)
{
    const UISkin* skin = ui_get_skin();
    if (skin->tooltip.pixels)
        ui_draw_nineslice(x, y, width, height, skin->tooltip.pixels, skin->tooltip.width, skin->tooltip.height, 8, 8, 8, 8);

    if (text)
        ui_draw_text_wrapped(x + 6, y + 6, width - 12, text, UI_COLOR_BLACK);
}

void ui_notification(int32_t x, int32_t y, int32_t width, int32_t height, const char* text)
{
    const UISkin* skin = ui_get_skin();
    if (skin->notification.pixels)
        ui_draw_nineslice(x, y, width, height, skin->notification.pixels, skin->notification.width, skin->notification.height, 8, 8, 8, 8);

    if (text)
    {
        int32_t text_w = ui_text_width(text);
        int32_t text_x = x + (width - text_w) / 2;
        int32_t text_y = y + (height - ui_text_height(text, width)) / 2;
        ui_draw_text_colored(text_x, text_y, text, UI_COLOR_BLACK);
    }
}

void ui_cursor(int32_t x, int32_t y)
{
    const UISkin* skin = ui_get_skin();
    if (skin->cursor.pixels)
        ui_draw_nineslice(x, y, skin->cursor.width, skin->cursor.height, skin->cursor.pixels, skin->cursor.width, skin->cursor.height, 0, 0, 0, 0);
}