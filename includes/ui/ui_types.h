// ui_types.h
#ifndef UI_TYPES_H
#define UI_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define UI_COLOR_BLACK      0xFF000000
#define UI_COLOR_WHITE      0xFFFFFFFF
#define UI_COLOR_GRAY       0xFF808080
#define UI_COLOR_DARK_GRAY  0xFF404040

typedef struct
{
    int mouse_x, mouse_y;
    bool mouse_down;
    bool nav_activate;
    int hot_item;
    int active_item;
    int next_id;
    int focused_id;
    bool nav_mode;
    int clip_x, clip_y;
    int clip_w, clip_h;
} UIContext;

typedef struct
{
    float x;
    float y;
} UIScrollValue;

extern UIContext g_ui;

#endif // !UI_TYPES_H