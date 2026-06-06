#include "ui/ui_skin.h"
#include <stddef.h>

// Linker-embedded skin binaries
extern const uint8_t _binary_data_ui_skins_default_gbskin_start[] 	__asm__("_binary_data_ui_skins_default_gbskin_start");
extern const uint8_t _binary_data_ui_skins_default_gbskin_end[] 	__asm__("_binary_data_ui_skins_default_gbskin_end");

static UISkin     g_skins[SKIN_COUNT];
// static uint32_t*  g_skin_palettes[SKIN_COUNT][256]; // max 256 palette variants per skin
static const UISkin* g_current_skin = NULL;

// ----------------------------------------------------------------
// .gbskin reader
// ----------------------------------------------------------------

#define GBSKIN_MAGIC 0x49554247 // 'GBUI' little-endian

static void skin_load(UISkin* skin, const uint8_t* data)
{
    const uint8_t* p = data;

    // Magic
    uint32_t magic = *(const uint32_t*)p; p += 4;
    if (magic != GBSKIN_MAGIC) return;

    // Palette count
    uint32_t palette_count = *(const uint32_t*)p; p += 4;
    skin->palette_count = palette_count;

    // Palette pointers — point directly into the binary
    static const uint32_t* palette_ptrs[256];
    for (uint32_t i = 0; i < palette_count && i < 256; i++)
    {
        palette_ptrs[i] = (const uint32_t*)p;
        p += 256 * sizeof(uint32_t);
    }

    skin->palettes      = palette_ptrs;
    skin->active_palette = palette_ptrs[0];

    // Elements — in fixed order matching Pigment's export
    Element* elements[] = {
        &skin->panel,
        &skin->window,
        &skin->tooltip,
        &skin->dialogue_box,
        &skin->portrait_frame,
        &skin->notification,
        &skin->button,
        &skin->button_hot,
        &skin->button_active,
        &skin->item_slot,
        &skin->item_slot_hot,
        &skin->item_slot_active,
        &skin->cursor,
        &skin->row_highlight,
        &skin->scrollbar_track,
        &skin->scrollbar_thumb,
        &skin->scrollbar_thumb_hot,
        &skin->bar_track,
        &skin->bar_fill,
        &skin->checkbox,
        &skin->checkbox_checked,
    };

    int element_count = sizeof(elements) / sizeof(elements[0]);

    for (int i = 0; i < element_count; i++)
    {
        uint32_t w = *(const uint32_t*)p; p += 4;
        uint32_t h = *(const uint32_t*)p; p += 4;

        elements[i]->width  = w;
        elements[i]->height = h;

        if (w > 0 && h > 0)
        {
            elements[i]->pixels = (const uint32_t*)p;
            p += w * h * sizeof(uint32_t);
        }
        else
        {
            elements[i]->pixels = NULL;
        }
    }
}

// ----------------------------------------------------------------
// Public API
// ----------------------------------------------------------------

void ui_skins_init(void)
{
    skin_load(&g_skins[SKIN_GLYPHBORN], _binary_data_ui_skins_default_gbskin_start);

    g_current_skin = &g_skins[SKIN_GLYPHBORN];
}

const UISkin* ui_get_skin(void)
{
    return g_current_skin;
}

void ui_set_skin(Skin skin)
{
    if (skin >= 0 && skin < SKIN_COUNT)
        g_current_skin = &g_skins[skin];
}

void ui_set_palette(Skin skin, uint32_t palette_index)
{
    if (skin < 0 || skin >= SKIN_COUNT) return;
    if (palette_index >= g_skins[skin].palette_count) return;
    g_skins[skin].active_palette = g_skins[skin].palettes[palette_index];
}