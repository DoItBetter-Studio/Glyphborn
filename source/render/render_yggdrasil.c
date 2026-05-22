#ifdef __YGGDRASIL__
#include "render.h"
#include <yggdrasil.h>

/* Game-side framebuffer storage — always 1280x720 internally */
uint32_t framebuffer     [FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_game[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_ui  [FB_WIDTH * FB_HEIGHT];
float    depthbuffer     [FB_WIDTH * FB_HEIGHT];

static YggFramebuffer *g_fb = NULL;

/* ── Minimal debug sprintf — supports %u, %d, %s, %% only ── */
static void dbg_uint(char *buf, int *pos, unsigned int v) {
    char tmp[12];
    int  len = 0;
    if (v == 0) { tmp[len++] = '0'; }
    while (v > 0) { tmp[len++] = '0' + (v % 10); v /= 10; }
    for (int i = len - 1; i >= 0; i--) buf[(*pos)++] = tmp[i];
}

static int dbg_sprintf(char *buf, const char *fmt, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    int pos = 0;
    for (; *fmt; fmt++) {
        if (*fmt != '%') { buf[pos++] = *fmt; continue; }
        fmt++;
        if (*fmt == 'u') {
            dbg_uint(buf, &pos, __builtin_va_arg(ap, unsigned int));
        } else if (*fmt == 'd') {
            int v = __builtin_va_arg(ap, int);
            if (v < 0) { buf[pos++] = '-'; v = -v; }
            dbg_uint(buf, &pos, (unsigned int)v);
        } else if (*fmt == 's') {
            const char *s = __builtin_va_arg(ap, const char *);
            while (*s) buf[pos++] = *s++;
        } else if (*fmt == '%') {
            buf[pos++] = '%';
        }
    }
    buf[pos] = '\0';
    __builtin_va_end(ap);
    return pos;
}

/* ── Pixel font renderer — no SDK dependency ── */
/* 4x6 digits 0-9 and basic chars, 1bpp packed rows */
static const uint8_t dbg_font[11][6] = {
    {0x6,0x9,0x9,0x9,0x9,0x6}, /* 0 */
    {0x2,0x6,0x2,0x2,0x2,0x7}, /* 1 */
    {0x6,0x9,0x1,0x2,0x4,0xF}, /* 2 */
    {0x6,0x9,0x2,0x1,0x9,0x6}, /* 3 */
    {0x1,0x3,0x5,0xF,0x1,0x1}, /* 4 */
    {0xF,0x8,0xE,0x1,0x9,0x6}, /* 5 */
    {0x6,0x8,0xE,0x9,0x9,0x6}, /* 6 */
    {0xF,0x1,0x2,0x4,0x4,0x4}, /* 7 */
    {0x6,0x9,0x6,0x9,0x9,0x6}, /* 8 */
    {0x6,0x9,0x9,0x7,0x1,0x6}, /* 9 */
    {0x0,0x0,0x0,0x0,0x0,0x0}, /* space / unknown */
};

static void dbg_draw_char(uint32_t *dst, uint32_t pitch_words,
                           uint32_t x, uint32_t y,
                           char c, uint32_t color, uint32_t bg) {
    int idx = (c >= '0' && c <= '9') ? c - '0' : 10;
    for (int row = 0; row < 6; row++)
        for (int col = 0; col < 4; col++)
            dst[(y + row) * pitch_words + (x + col)] =
                (dbg_font[idx][row] & (0x8 >> col)) ? color : bg;
}

static void dbg_draw_string(uint32_t *dst, uint32_t pitch_words,
                             uint32_t x, uint32_t y,
                             const char *s, uint32_t color, uint32_t bg) {
    for (; *s; s++, x += 5)
        dbg_draw_char(dst, pitch_words, x, y, *s, color, bg);
}

void render_init(void *platform_context) {
    g_fb = (YggFramebuffer *)platform_context;
}

uint32_t *render_get_framebuffer(void) {
    return framebuffer;
}

void render_clear(uint32_t *buffer, uint32_t color) {
    for (int i = 0; i < FB_WIDTH * FB_HEIGHT; i++)
        buffer[i] = color;
}

void render_blend_ui_over_game(void) {
    for (int i = 0; i < FB_WIDTH * FB_HEIGHT; i++) {
        uint32_t ui   = framebuffer_ui[i];
        uint8_t  a    = (ui >> 24) & 0xFF;

        if (a == 0) {
            framebuffer[i] = framebuffer_game[i];
        } else if (a == 255) {
            framebuffer[i] = ui;
        } else {
            uint32_t game = framebuffer_game[i];
            uint8_t ru = (ui   >> 16) & 0xFF, gu = (ui   >> 8) & 0xFF, bu = ui   & 0xFF;
            uint8_t rg = (game >> 16) & 0xFF, gg = (game >> 8) & 0xFF, bg = game & 0xFF;
            uint8_t r  = (ru * a + rg * (255 - a)) / 255;
            uint8_t g2 = (gu * a + gg * (255 - a)) / 255;
            uint8_t b  = (bu * a + bg * (255 - a)) / 255;
            framebuffer[i] = (0xFF << 24) | (r << 16) | (g2 << 8) | b;
        }
    }
}

void render_present(void) {
    if (!g_fb || !g_fb->pixels) return;

    uint32_t *dst         = g_fb->pixels;
    uint32_t  pitch_words = g_fb->pitch / 4;
    uint32_t  kw          = g_fb->width;
    uint32_t  kh          = g_fb->height;

    for (uint32_t y = 0; y < kh; y++) {
        uint32_t src_y = (y * FB_HEIGHT) / kh;
        for (uint32_t x = 0; x < kw; x++) {
            uint32_t src_x = (x * FB_WIDTH) / kw;
            dst[y * pitch_words + x] = framebuffer[src_y * FB_WIDTH + src_x];
        }
    }

    /* DEBUG OVERLAY — remove once values confirmed */
    char buf[64];
    dbg_sprintf(buf, "W%u H%u P%u BPP%u", kw, kh, g_fb->pitch, g_fb->bpp);
    dbg_draw_string(dst, pitch_words, 4, 4, buf, 0xFFFFFF00, 0xFF000000);
}

void render_shutdown(void) {
    g_fb = NULL;
}
#endif /* __YGGDRASIL__ */