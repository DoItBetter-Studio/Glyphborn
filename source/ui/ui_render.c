#include "ui/ui_render.h"
#include "ui/ui.h" // Needed for g_ui clipping bounds
#include "font/ascii_tileset.h"

void ui_draw_image(int32_t x, int32_t y, int32_t width, int32_t height, int32_t depth, const unsigned char* image_data, const unsigned char* palette)
{
    int32_t mask = (1 << depth) - 1;
    int32_t total_pixels = width * height;
    int32_t data_index = 0;

    for (int32_t j = 0; j < height && (y + j) < FB_HEIGHT; ++j)
    {
        for (int32_t i = 0; i < width && (x + i) < FB_WIDTH; ++i)
        {
            if (data_index >= total_pixels) break;

            int32_t pixel_byte_index = (data_index * depth) / 8;
            int32_t bit_shift = 8 - depth - ((data_index * depth) % 8);

            uint8_t byte = image_data[pixel_byte_index];
            uint8_t index = (byte >> bit_shift) & mask;

            if (index != 0)
            {
                uint8_t r = palette[index * 3 + 0];
                uint8_t g = palette[index * 3 + 1];
                uint8_t b = palette[index * 3 + 2];

                uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
                framebuffer_ui[(y + j) * FB_WIDTH + (x + i)] = color;
            }
            data_index++;
        }
    }
}

void ui_draw_text(int32_t x, int32_t y, const char* text)
{
    ui_draw_text_colored(x, y, text, 0xFF000000);
}

void ui_draw_text_colored(int32_t x, int32_t y, const char* text, uint32_t color)
{
    int32_t cursor_x = x;
    int32_t cursor_y = y;

    while (*text)
    {
        char c = *text++;
        if (c == '\n')
        {
            cursor_x = x;
            cursor_y += ASCII_TILESET_GLYPH_HEIGHT;
            continue;
        }

        int32_t glyph_index = (unsigned char)c;
        int32_t glyph_col = glyph_index % ASCII_TILESET_SHEET_COLUMNS;
        int32_t glyph_row = glyph_index / ASCII_TILESET_SHEET_COLUMNS;

        const int32_t sheet_width = ASCII_TILESET_SHEET_COLUMNS * ASCII_TILESET_GLYPH_WIDTH;
        const int32_t mask = (1 << ASCII_TILESET_BITDEPTH) - 1;
        int32_t draw_width = ascii_tileset_widths[(unsigned char)c];

        for (int32_t j = 0; j < ASCII_TILESET_GLYPH_HEIGHT && (cursor_y + j) < FB_HEIGHT; ++j)
        {
            for (int32_t i = 0; i < draw_width && (cursor_x + i) < FB_WIDTH; ++i)
            {
                int32_t pixel_index = j * sheet_width + i + glyph_col * ASCII_TILESET_GLYPH_WIDTH + glyph_row * sheet_width * ASCII_TILESET_GLYPH_HEIGHT;
                int32_t bit_index = pixel_index * ASCII_TILESET_BITDEPTH;
                int32_t byte_index = bit_index / 8;
                int32_t bit_shift = 8 - ASCII_TILESET_BITDEPTH - (bit_index % 8);

                uint8_t byte = ascii_tileset[byte_index];
                uint8_t index = (byte >> bit_shift) & mask;

                if (index == 0) continue;

                framebuffer_ui[(cursor_y + j) * FB_WIDTH + (cursor_x + i)] = color;
            }
        }
        cursor_x += draw_width + ASCII_TILESET_SPACING;
    }
}

void ui_draw_text_wrapped(int32_t x, int32_t y, int32_t max_width, const char* text, uint32_t color)
{
    int32_t cursor_x = x;
    int32_t cursor_y = y;
    const int32_t sheet_width = ASCII_TILESET_SHEET_COLUMNS * ASCII_TILESET_GLYPH_WIDTH;
    const int32_t mask = (1 << ASCII_TILESET_BITDEPTH) - 1;

    while (*text)
    {
        char c = *text++;
        if (c == '\n')
        {
            cursor_x = x;
            cursor_y += ASCII_TILESET_GLYPH_HEIGHT;
            continue;
        }

        int32_t glyph_w = ascii_tileset_widths[(unsigned char)c] + ASCII_TILESET_SPACING;
        if (cursor_x + glyph_w > x + max_width)
        {
            cursor_x = x;
            cursor_y += ASCII_TILESET_GLYPH_HEIGHT;
        }

        int32_t glyph_index = (unsigned char)c;
        int32_t glyph_col   = glyph_index % ASCII_TILESET_SHEET_COLUMNS;
        int32_t glyph_row   = glyph_index / ASCII_TILESET_SHEET_COLUMNS;
        int32_t draw_width = ascii_tileset_widths[(unsigned char)c];

        int32_t clip_x2 = g_ui.clip_x + g_ui.clip_w;
        int32_t clip_y2 = g_ui.clip_y + g_ui.clip_h;

        for (int32_t j = 0; j < ASCII_TILESET_GLYPH_HEIGHT && (cursor_y + j) < FB_HEIGHT; ++j)
        {
            if ((cursor_y + j) < g_ui.clip_y || (cursor_y + j) >= clip_y2) continue;

            for (int32_t i = 0; i < draw_width && (cursor_x + i) < FB_WIDTH; ++i)
            {
                if ((cursor_x + i) < g_ui.clip_x || (cursor_x + i) >= clip_x2) continue;

                int32_t pixel_index = j * sheet_width + i + glyph_col * ASCII_TILESET_GLYPH_WIDTH + glyph_row * sheet_width * ASCII_TILESET_GLYPH_HEIGHT;
                int32_t bit_index  = pixel_index * ASCII_TILESET_BITDEPTH;
                int32_t byte_index = bit_index / 8;
                int32_t bit_shift  = 8 - ASCII_TILESET_BITDEPTH - (bit_index % 8);

                uint8_t byte  = ascii_tileset[byte_index];
                uint8_t index = (byte >> bit_shift) & mask;

                if (index == 0) continue;

                framebuffer_ui[(cursor_y + j) * FB_WIDTH + (cursor_x + i)] = color;
            }
        }
        cursor_x += glyph_w;
    }
}

int32_t ui_text_width(const char* text)
{
    int32_t label_width = 0;
    for (const char* c = text; *c; ++c)
        label_width += ascii_tileset_widths[(unsigned char)*c] + ASCII_TILESET_SPACING;
    return label_width;
}

int32_t ui_text_height(const char* text, int32_t max_width)
{
    int32_t line_width = 0;
    int32_t total_height = ASCII_TILESET_GLYPH_HEIGHT;

    for (const char* c = text; *c; ++c)
    {
        if (*c == '\n')
        {
            line_width = 0;
            total_height += ASCII_TILESET_GLYPH_HEIGHT;
            continue;
        }

        int32_t glyph_width = ascii_tileset_widths[(unsigned char)*c] + ASCII_TILESET_SPACING;
        if (line_width + glyph_width > max_width)
        {
            line_width = 0;
            total_height += ASCII_TILESET_GLYPH_HEIGHT;
        }
        line_width += glyph_width;
    }
    return total_height;
}

static void blit_tiled_region(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
    int32_t src_x, int32_t src_y, int32_t src_w, int32_t src_h,
    const uint32_t* pixels, int32_t image_w, int32_t image_h)
{
    int32_t clip_x2 = g_ui.clip_x + g_ui.clip_w;
    int32_t clip_y2 = g_ui.clip_y + g_ui.clip_h;

    for (int32_t dy = 0; dy < dst_h; ++dy)
    {
        for (int32_t dx = 0; dx < dst_w; ++dx)
        {
            int32_t tx = dx % src_w;
            int32_t ty = dy % src_h;
            int32_t src_px_x = src_x + tx;
            int32_t src_px_y = src_y + ty;

            if (src_px_x >= image_w || src_px_y >= image_h) continue;

            uint32_t pixel = pixels[src_px_y * image_w + src_px_x];
            if ((pixel >> 24) == 0) continue;

            int32_t dst_px_x = dst_x + dx;
            int32_t dst_px_y = dst_y + dy;

            if (dst_px_x < g_ui.clip_x || dst_px_x >= clip_x2 ||
                dst_px_y < g_ui.clip_y || dst_px_y >= clip_y2) continue;

            if (dst_px_x < 0 || dst_px_x >= FB_WIDTH ||
                dst_px_y < 0 || dst_px_y >= FB_HEIGHT) continue;

            framebuffer_ui[dst_px_y * FB_WIDTH + dst_px_x] = pixel;
        }
    }
}

void ui_draw_nineslice(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
    const uint32_t* pixels, int32_t src_w, int32_t src_h,
    int32_t slice_left, int32_t slice_top, int32_t slice_right, int32_t slice_bottom)
{
    int32_t center_src_w = src_w - slice_left - slice_right;
    int32_t center_src_h = src_h - slice_top - slice_bottom;
    int32_t center_dst_w = dst_w - slice_left - slice_right;
    int32_t center_dst_h = dst_h - slice_top - slice_bottom;

    // Corners
    blit_tiled_region(dst_x,                    dst_y,                    slice_left,  slice_top,    0,                    0,                    slice_left,  slice_top,    pixels, src_w, src_h);
    blit_tiled_region(dst_x + dst_w - slice_right, dst_y,                 slice_right, slice_top,    src_w - slice_right,  0,                    slice_right, slice_top,    pixels, src_w, src_h);
    blit_tiled_region(dst_x,                    dst_y + dst_h - slice_bottom, slice_left, slice_bottom, 0,                 src_h - slice_bottom, slice_left,  slice_bottom, pixels, src_w, src_h);
    blit_tiled_region(dst_x + dst_w - slice_right, dst_y + dst_h - slice_bottom, slice_right, slice_bottom, src_w - slice_right, src_h - slice_bottom, slice_right, slice_bottom, pixels, src_w, src_h);

    // Edges
    blit_tiled_region(dst_x + slice_left,         dst_y,                    center_dst_w, slice_top,    slice_left,           0,                    center_src_w, slice_top,    pixels, src_w, src_h);
    blit_tiled_region(dst_x + slice_left,         dst_y + dst_h - slice_bottom, center_dst_w, slice_bottom, slice_left,      src_h - slice_bottom, center_src_w, slice_bottom, pixels, src_w, src_h);
    blit_tiled_region(dst_x,                    dst_y + slice_top,          slice_left,  center_dst_h, 0,                    slice_top,            slice_left,  center_src_h, pixels, src_w, src_h);
    blit_tiled_region(dst_x + dst_w - slice_right, dst_y + slice_top,      slice_right, center_dst_h, src_w - slice_right,  slice_top,            slice_right, center_src_h, pixels, src_w, src_h);

    // Center
    blit_tiled_region(dst_x + slice_left, dst_y + slice_top, center_dst_w, center_dst_h, slice_left, slice_top, center_src_w, center_src_h, pixels, src_w, src_h);
}

void ui_set_clip(int32_t x, int32_t y, int32_t width, int32_t height)
{
    g_ui.clip_x = x;
    g_ui.clip_y = y;
    g_ui.clip_w = width;
    g_ui.clip_h = height;
}

void ui_reset_clip(void)
{
    g_ui.clip_x = 0;
    g_ui.clip_y = 0;
    g_ui.clip_w = FB_WIDTH;
    g_ui.clip_h = FB_HEIGHT;
}