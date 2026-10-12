#include "ui/ui_render.h"
#include "ui/ui.h" // Needed for g_ui clipping bounds
#include "font/ascii_tileset.h"
#include "font/cjk_tileset_ja_jp.h"
#include "font/cjk_tileset_ko_kr.h"
#include "font/cjk_tileset_zh_cn.h"
#include "font/cjk_tileset_zh_tw.h"

static LocaleIndex g_current_locale = LOCALE_EN_US;

static inline int32_t map_codepoint_to_tile(uint32_t cp)
{
    // 1. Basic ASCII (0x0020 - 0x007E) -> Slots 32 to 126
    // (Slot 0-31 are printable glyphs in your image: space, !, ", #, etc.)
    if (cp <= 0x007F)
        return (int32_t)cp;

    // 2. Latin-1 Supplement (0x00C0 - 0x00FF) -> Slots 128 to 191
    if (cp >= 0x00C0 && cp <= 0x00FF)
        return 128 + (int32_t)(cp - 0x00C0);

    // 3. Latin Extended-A (0x0100 - 0x017F) -> Slots 192 to 319
    if (cp >= 0x0100 && cp <= 0x017F)
        return 192 + (int32_t)(cp - 0x0100);

    // 4. Cyrillic Uppercase Extras (0x0401 - 0x040F) -> Slots 320 to 334
    // (Ё, Ђ, Ѓ, Є, Ѕ, І, Ї, Ј, Љ, Њ, Ћ, Ќ, Ѝ, Ў, Џ)
    if (cp >= 0x0401 && cp <= 0x040F)
        return 320 + (int32_t)(cp - 0x0401);

    // 5. Cyrillic Main Block (0x0410 - 0x044F) -> Slots 335 to 398
    // (А-Я, а-я)
    if (cp >= 0x0410 && cp <= 0x044F)
        return 335 + (int32_t)(cp - 0x0410);

    // 6. Cyrillic Lowercase Extras (0x0450 - 0x045F) -> Slots 399 to 414
    // (ѐ, ё, ђ, ѓ, є, ѕ, і, ї, ј, љ, њ, ћ, ќ, ѝ, ў, џ)
    if (cp >= 0x0450 && cp <= 0x045F)
        return 399 + (int32_t)(cp - 0x0450);

    // Fallback glyph slot (e.g., '?' or 0x003F)
    return '?';
}

static inline bool is_cjk_codepoint(uint32_t cp)
{
    return (cp >= 0x3000 && cp <= 0x303F) || // CJK Symbols and Punctuation
           (cp >= 0x3040 && cp <= 0x309F) || // Hiragana
           (cp >= 0x30A0 && cp <= 0x30FF) || // Katakana
           (cp >= 0x3400 && cp <= 0x4DBF) || // CJK Extension A
           (cp >= 0x4E00 && cp <= 0x9FFF) || // CJK Unified Ideographs
           (cp >= 0xAC00 && cp <= 0xD7AF) || // Hangul Syllables
           (cp >= 0xF900 && cp <= 0xFAFF) || // CJK Compatibility Ideographs
           (cp >= 0xFF00 && cp <= 0xFFEF);   // Halfwidth and Fullwidth Forms
}

#define DEFINE_CJK_LOOKUP(locale_name, prefix, count_macro)               \
    static uint16_t map_codepoint_to_cjk_tile_##locale_name(uint32_t cp)  \
    {                                                                      \
        int low = 0;                                                       \
        int high = count_macro - 1;                                        \
        while (low <= high)                                                \
        {                                                                  \
            int mid = low + (high - low) / 2;                              \
            if (prefix##_glyphs[mid].codepoint == cp)                      \
                return prefix##_glyphs[mid].tile;       /* Fixed field */  \
            if (prefix##_glyphs[mid].codepoint < cp)                       \
                low = mid + 1;                                             \
            else                                                           \
                high = mid - 1;                                            \
        }                                                                  \
        return 0; /* Fallback tile index */                                \
    }

DEFINE_CJK_LOOKUP(ja_jp, cjk_ja_jp, CJK_JA_JP_GLYPH_COUNT)
DEFINE_CJK_LOOKUP(ko_kr, cjk_ko_kr, CJK_KO_KR_GLYPH_COUNT)
DEFINE_CJK_LOOKUP(zh_cn, cjk_zh_cn, CJK_ZH_CN_GLYPH_COUNT)
DEFINE_CJK_LOOKUP(zh_tw, cjk_zh_tw, CJK_ZH_TW_GLYPH_COUNT)

static inline uint32_t decode_utf8_next(const unsigned char **p)
{
    uint32_t codepoint = **p;
    (*p)++;

    if ((codepoint & 0x80) != 0)
    {
        if ((codepoint & 0xE0) == 0xC0 && **p)
        {
            codepoint = ((codepoint & 0x1F) << 6) | (**p & 0x3F);
            (*p)++;
        }
        else if ((codepoint & 0xF0) == 0xE0 && (*p)[0] && (*p)[1])
        {
            codepoint = ((codepoint & 0x0F) << 12) | (((*p)[0] & 0x3F) << 6) | ((*p)[1] & 0x3F);
            *p += 2;
        }
    }

    return codepoint;
}

typedef struct
{
    const unsigned char *tileset;
    const uint8_t *widths;

    int32_t glyph_width;
    int32_t glyph_height;
    int32_t sheet_columns;

    int32_t bitdepth;
    int32_t spacing;

    int32_t tile_index;
} UiGlyph;

static inline UiGlyph get_glyph(uint32_t cp)
{
    if (is_cjk_codepoint(cp))
    {
        switch (g_current_locale)
        {
            case LOCALE_JA_JP:
                return (UiGlyph){
                    .tileset = cjk_ja_jp_tileset,
                    .widths = cjk_ja_jp_tileset_widths,
                    .glyph_width = CJK_TILESET_JA_JP_GLYPH_WIDTH,
                    .glyph_height = CJK_TILESET_JA_JP_GLYPH_HEIGHT,
                    .sheet_columns = CJK_TILESET_JA_JP_SHEET_COLUMNS,
                    .bitdepth = CJK_TILESET_JA_JP_BITDEPTH,
                    .spacing = CJK_TILESET_JA_JP_SPACING,
                    .tile_index = map_codepoint_to_cjk_tile_ja_jp(cp)};

            case LOCALE_ZH_CN:
                return (UiGlyph){
                    .tileset = cjk_zh_cn_tileset,
                    .widths = cjk_zh_cn_tileset_widths,
                    .glyph_width = CJK_TILESET_ZH_CN_GLYPH_WIDTH,
                    .glyph_height = CJK_TILESET_ZH_CN_GLYPH_HEIGHT,
                    .sheet_columns = CJK_TILESET_ZH_CN_SHEET_COLUMNS,
                    .bitdepth = CJK_TILESET_ZH_CN_BITDEPTH,
                    .spacing = CJK_TILESET_ZH_CN_SPACING,
                    .tile_index = map_codepoint_to_cjk_tile_zh_cn(cp)};

            case LOCALE_KO_KR:
                return (UiGlyph){
                    .tileset = cjk_ko_kr_tileset,
                    .widths = cjk_ko_kr_tileset_widths,
                    .glyph_width = CJK_TILESET_KO_KR_GLYPH_WIDTH,
                    .glyph_height = CJK_TILESET_KO_KR_GLYPH_HEIGHT,
                    .sheet_columns = CJK_TILESET_KO_KR_SHEET_COLUMNS,
                    .bitdepth = CJK_TILESET_KO_KR_BITDEPTH,
                    .spacing = CJK_TILESET_KO_KR_SPACING,
                    .tile_index = map_codepoint_to_cjk_tile_ko_kr(cp)};

            case LOCALE_ZH_TW:
                return (UiGlyph){
                    .tileset = cjk_zh_tw_tileset,
                    .widths = cjk_zh_tw_tileset_widths,
                    .glyph_width = CJK_TILESET_ZH_TW_GLYPH_WIDTH,
                    .glyph_height = CJK_TILESET_ZH_TW_GLYPH_HEIGHT,
                    .sheet_columns = CJK_TILESET_ZH_TW_SHEET_COLUMNS,
                    .bitdepth = CJK_TILESET_ZH_TW_BITDEPTH,
                    .spacing = CJK_TILESET_ZH_TW_SPACING,
                    .tile_index = map_codepoint_to_cjk_tile_zh_tw(cp)};

            default:
                break;
        }
    }

    return (UiGlyph){
        .tileset = ascii_tileset,
        .widths = ascii_tileset_widths,
        .glyph_width = ASCII_TILESET_GLYPH_WIDTH,
        .glyph_height = ASCII_TILESET_GLYPH_HEIGHT,
        .sheet_columns = ASCII_TILESET_SHEET_COLUMNS,
        .bitdepth = ASCII_TILESET_BITDEPTH,
        .spacing = ASCII_TILESET_SPACING,
        .tile_index = map_codepoint_to_tile(cp)};
}

void ui_set_locale(LocaleIndex locale)
{
    g_current_locale = locale;
}

void ui_draw_image(int32_t x, int32_t y, int32_t width, int32_t height, int32_t depth, const unsigned char *image_data, const unsigned char *palette)
{
    int32_t mask = (1 << depth) - 1;
    int32_t total_pixels = width * height;
    int32_t data_index = 0;

    for (int32_t j = 0; j < height && (y + j) < FB_HEIGHT; ++j)
    {
        for (int32_t i = 0; i < width && (x + i) < FB_WIDTH; ++i)
        {
            if (data_index >= total_pixels)
                break;

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

void ui_draw_text(int32_t x, int32_t y, const char *text)
{
    ui_draw_text_colored(x, y, text, 0xFF000000);
}

void ui_draw_text_colored(int32_t x, int32_t y, const char *text, uint32_t color)
{
    int32_t cursor_x = x;
    int32_t cursor_y = y;

    const unsigned char *p = (const unsigned char *)text;

    while (*p)
    {
        uint32_t codepoint = decode_utf8_next(&p);
        UiGlyph glyph = get_glyph(codepoint);

        if (codepoint == '\n')
        {
            cursor_x = x;
            cursor_y += glyph.glyph_height; // Dynamic line advancement
            continue;
        }

        int32_t glyph_col = glyph.tile_index % glyph.sheet_columns;
        int32_t glyph_row = glyph.tile_index / glyph.sheet_columns;
        int32_t sheet_width = glyph.sheet_columns * glyph.glyph_width;
        int32_t draw_width = glyph.widths[glyph.tile_index];

        int32_t clip_x2 = g_ui.clip_x + g_ui.clip_w;
        int32_t clip_y2 = g_ui.clip_y + g_ui.clip_h;

        for (int32_t j = 0; j < glyph.glyph_height && (cursor_y + j) < FB_HEIGHT; ++j)
        {
            if ((cursor_y + j) < g_ui.clip_y || (cursor_y + j) >= clip_y2)
                continue;

            for (int32_t i = 0; i < draw_width && (cursor_x + i) < FB_WIDTH; ++i)
            {
                if ((cursor_x + i) < g_ui.clip_x || (cursor_x + i) >= clip_x2)
                    continue;

                int32_t pixel_index =
                    j * sheet_width +
                    i +
                    glyph_col * glyph.glyph_width +
                    glyph_row * sheet_width * glyph.glyph_height;

                int32_t bit_index = pixel_index * glyph.bitdepth;
                int32_t byte_index = bit_index / 8;
                int32_t bit_shift = 8 - glyph.bitdepth - (bit_index % 8);

                uint8_t byte = glyph.tileset[byte_index];
                uint8_t index = (byte >> bit_shift) & ((1 << glyph.bitdepth) - 1);

                if (index == 0)
                    continue;

                framebuffer_ui[(cursor_y + j) * FB_WIDTH + (cursor_x + i)] = color;
            }
        }

        cursor_x += draw_width + glyph.spacing;
    }
}

void ui_draw_text_wrapped(int32_t x, int32_t y, int32_t max_width,
                          const char *text, uint32_t color)
{
    int32_t cursor_x = x;
    int32_t cursor_y = y;

    const unsigned char *p = (const unsigned char *)text;

    while (*p)
    {
        // Word boundary check...
        if (p == (const unsigned char *)text || *(p - 1) == ' ' || *(p - 1) == '\n')
        {
            int32_t word_pixel_width = 0;
            int32_t word_max_height = ASCII_TILESET_GLYPH_HEIGHT;
            const unsigned char *lookahead = p;

            while (*lookahead && *lookahead != ' ' && *lookahead != '\n')
            {
                uint32_t cp = decode_utf8_next(&lookahead);
                UiGlyph glyph = get_glyph(cp);
                word_pixel_width += glyph.widths[glyph.tile_index] + glyph.spacing;
                if (glyph.glyph_height > word_max_height)
                    word_max_height = glyph.glyph_height;
            }

            if (cursor_x + word_pixel_width > x + max_width && cursor_x > x)
            {
                cursor_x = x;
                cursor_y += word_max_height;
            }
        }

        uint32_t codepoint = decode_utf8_next(&p);
        UiGlyph glyph = get_glyph(codepoint);

        if (codepoint == '\n')
        {
            cursor_x = x;
            cursor_y += glyph.glyph_height;
            continue;
        }

        int32_t glyph_col = glyph.tile_index % glyph.sheet_columns;
        int32_t glyph_row = glyph.tile_index / glyph.sheet_columns;
        int32_t sheet_width = glyph.sheet_columns * glyph.glyph_width;
        int32_t draw_width = glyph.widths[glyph.tile_index];

        int32_t clip_x2 = g_ui.clip_x + g_ui.clip_w;
        int32_t clip_y2 = g_ui.clip_y + g_ui.clip_h;

        for (int32_t j = 0; j < glyph.glyph_height && (cursor_y + j) < FB_HEIGHT; ++j)
        {
            if ((cursor_y + j) < g_ui.clip_y || (cursor_y + j) >= clip_y2)
                continue;

            for (int32_t i = 0; i < draw_width && (cursor_x + i) < FB_WIDTH; ++i)
            {
                if ((cursor_x + i) < g_ui.clip_x || (cursor_x + i) >= clip_x2)
                    continue;

                int32_t pixel_index =
                    j * sheet_width +
                    i +
                    glyph_col * glyph.glyph_width +
                    glyph_row * sheet_width * glyph.glyph_height;

                int32_t bit_index = pixel_index * glyph.bitdepth;
                int32_t byte_index = bit_index / 8;
                int32_t bit_shift = 8 - glyph.bitdepth - (bit_index % 8);

                uint8_t byte = glyph.tileset[byte_index];
                uint8_t index = (byte >> bit_shift) & ((1 << glyph.bitdepth) - 1);

                if (index == 0)
                    continue;

                framebuffer_ui[(cursor_y + j) * FB_WIDTH + (cursor_x + i)] = color;
            }
        }

        cursor_x += draw_width + glyph.spacing;
    }
}

int32_t ui_text_width(const char *text)
{
    int32_t label_width = 0;

    const unsigned char *p = (const unsigned char *)text;

    while (*p)
    {
        uint32_t cp = decode_utf8_next(&p);

        UiGlyph glyph = get_glyph(cp);

        label_width +=
            glyph.widths[glyph.tile_index] +
            glyph.spacing;
    }

    return label_width;
}

int32_t ui_text_height(const char *text, int32_t max_width)
{
    int32_t line_width = 0;
    int32_t total_height = ASCII_TILESET_GLYPH_HEIGHT;

    const unsigned char *p = (const unsigned char *)text;

    while (*p)
    {
        // Word boundary check for wrapping height
        if (p == (const unsigned char *)text ||
            *(p - 1) == ' ' ||
            *(p - 1) == '\n')
        {
            int32_t word_pixel_width = 0;
            int32_t word_height = ASCII_TILESET_GLYPH_HEIGHT;

            const unsigned char *lookahead = p;

            while (*lookahead &&
                   *lookahead != ' ' &&
                   *lookahead != '\n')
            {
                uint32_t cp = decode_utf8_next(&lookahead);

                UiGlyph glyph = get_glyph(cp);

                word_pixel_width +=
                    glyph.widths[glyph.tile_index] +
                    glyph.spacing;

                if (glyph.glyph_height > word_height)
                    word_height = glyph.glyph_height;
            }

            if (line_width + word_pixel_width > max_width &&
                line_width > 0)
            {
                line_width = 0;
                total_height += word_height;
            }
        }

        uint32_t cp = decode_utf8_next(&p);

        if (cp == '\n')
        {
            line_width = 0;
            total_height += ASCII_TILESET_GLYPH_HEIGHT;
            continue;
        }

        UiGlyph glyph = get_glyph(cp);

        line_width +=
            glyph.widths[glyph.tile_index] +
            glyph.spacing;
    }

    return total_height;
}

static void blit_tiled_region(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
                              int32_t src_x, int32_t src_y, int32_t src_w, int32_t src_h,
                              const uint8_t *pixels, int32_t image_w, int32_t image_h)
{
    const UISkin* skin = ui_get_skin();
    if (!skin || !skin->active_palette) return;

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

            if (src_px_x >= image_w || src_px_y >= image_h)
                continue;

            // Fetch the 8-bit index
            uint8_t index = pixels[src_px_y * image_w + src_px_x];
            
            // Index 0 or alpha lookup check
            uint32_t pixel = skin->active_palette[index];
            if ((pixel >> 24) == 0)
                continue;

            int32_t dst_px_x = dst_x + dx;
            int32_t dst_px_y = dst_y + dy;

            if (dst_px_x < g_ui.clip_x || dst_px_x >= clip_x2 ||
                dst_px_y < g_ui.clip_y || dst_px_y >= clip_y2)
                continue;

            if (dst_px_x < 0 || dst_px_x >= FB_WIDTH ||
                dst_px_y < 0 || dst_px_y >= FB_HEIGHT)
                continue;

            framebuffer_ui[dst_px_y * FB_WIDTH + dst_px_x] = pixel;
        }
    }
}

void ui_draw_nineslice(int32_t dst_x, int32_t dst_y, int32_t dst_w, int32_t dst_h,
                       const uint8_t* pixels, int32_t src_w, int32_t src_h,
                       int32_t slice_left, int32_t slice_top, int32_t slice_right, int32_t slice_bottom)
{
    if (!pixels || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return;

    int32_t center_src_w = src_w - slice_left - slice_right;
    int32_t center_src_h = src_h - slice_top - slice_bottom;
    int32_t center_dst_w = dst_w - slice_left - slice_right;
    int32_t center_dst_h = dst_h - slice_top - slice_bottom;

    // Corners
    blit_tiled_region(dst_x, dst_y, slice_left, slice_top, 0, 0, slice_left, slice_top, pixels, src_w, src_h);
    blit_tiled_region(dst_x + dst_w - slice_right, dst_y, slice_right, slice_top, src_w - slice_right, 0, slice_right, slice_top, pixels, src_w, src_h);
    blit_tiled_region(dst_x, dst_y + dst_h - slice_bottom, slice_left, slice_bottom, 0, src_h - slice_bottom, slice_left, slice_bottom, pixels, src_w, src_h);
    blit_tiled_region(dst_x + dst_w - slice_right, dst_y + dst_h - slice_bottom, slice_right, slice_bottom, src_w - slice_right, src_h - slice_bottom, slice_right, slice_bottom, pixels, src_w, src_h);

    // Edges
    blit_tiled_region(dst_x + slice_left, dst_y, center_dst_w, slice_top, slice_left, 0, center_src_w, slice_top, pixels, src_w, src_h);
    blit_tiled_region(dst_x + slice_left, dst_y + dst_h - slice_bottom, center_dst_w, slice_bottom, slice_left, src_h - slice_bottom, center_src_w, slice_bottom, pixels, src_w, src_h);
    blit_tiled_region(dst_x, dst_y + slice_top, slice_left, center_dst_h, 0, slice_top, slice_left, center_src_h, pixels, src_w, src_h);
    blit_tiled_region(dst_x + dst_w - slice_right, dst_y + slice_top, slice_right, center_dst_h, src_w - slice_right, slice_top, slice_right, center_src_h, pixels, src_w, src_h);

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