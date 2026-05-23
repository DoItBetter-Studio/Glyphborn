#ifndef UI_SKIN_H
#define UI_SKIN_H

#include <stdint.h>

typedef enum
{
    SKIN_GLYPHBORN,
    SKIN_SEER,
    SKIN_COUNT,
} Skin;

typedef struct Element
{
    const uint32_t* pixels;
    uint32_t width;
    uint32_t height;
} Element;

typedef struct UISkin
{
    const uint32_t*  active_palette;
    const uint32_t** palettes;
    uint32_t         palette_count;

    Element panel;
    Element window;
    Element tooltip;
    Element dialogue_box;
    Element portrait_frame;
    Element notification;

    Element button;
    Element button_hot;
    Element button_active;

    Element item_slot;
    Element item_slot_hot;
    Element item_slot_active;

    Element cursor;
    Element row_highlight;

    Element scrollbar_track;
    Element scrollbar_thumb;
    Element scrollbar_thumb_hot;

    Element bar_track;
    Element bar_fill;

    Element checkbox;
    Element checkbox_checked;

} UISkin;

const UISkin* ui_get_skin(void);
void          ui_set_skin(Skin skin);
void          ui_set_palette(Skin skin, uint32_t palette_index);
void          ui_skins_init(void);

#endif // !UI_SKIN_H