#ifdef __linux__
#include "input/input.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

typedef struct {
    Display* display;
    Window   window;
    GC       gc;
} X11Context;

static InputMapping button_mappings[BUTTON_COUNT] = {
    [BUTTON_UP]           = { XK_w,      0x01 },
    [BUTTON_DOWN]         = { XK_s,      0x02 },
    [BUTTON_LEFT]         = { XK_a,      0x04 },
    [BUTTON_RIGHT]        = { XK_d,      0x08 },
    [BUTTON_A]            = { XK_z,      0x10 },
    [BUTTON_B]            = { XK_x,      0x20 },
    [BUTTON_X]            = { XK_q,      0x40 },
    [BUTTON_Y]            = { XK_e,      0x80 },
    [BUTTON_LEFT_BUMPER]  = { XK_l,      0x100 },
    [BUTTON_RIGHT_BUMPER] = { XK_r,      0x200 },
    [BUTTON_START]        = { XK_Return, 0x400 },
    [BUTTON_SELECT]       = { XK_Tab,    0x800 },
};

static InputState input_state;
static Display* display = NULL;
static Window window = 0;
static int joystick_fd = -1;

void input_init(void* platform_context)
{
    memset(&input_state, 0, sizeof(InputState));

    if (platform_context)
    {
        X11Context* ctx = (X11Context*)platform_context;
        display = ctx->display;
        window  = ctx->window;
    }

    joystick_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
    if (joystick_fd < 0)
    {
        printf("Failed to open joystick device: %s\n", strerror(errno));
        joystick_fd = -1;
    }
}

void input_update(void)
{
    /* 1. Latch previous frame states */
    for (int i = 0; i < BUTTON_COUNT; ++i)
        input_state.button_prev[i] = input_state.button_down[i];

    input_state.mouse_prev = input_state.mouse_down;

    /* 2. Update X11 Keyboard & Mouse */
    if (display && window)
    {
        /* Query global keyboard map */
        char keys[32];
        XQueryKeymap(display, keys);

        for (int i = 0; i < BUTTON_COUNT; ++i)
        {
            KeySym keysym = button_mappings[i].keyboard_key;
            KeyCode keycode = XKeysymToKeycode(display, keysym);
            
            if (keycode != 0)
            {
                bool key_down = (keys[keycode / 8] & (1 << (keycode % 8))) != 0;
                input_state.button_down[i] = key_down;
            }
        }

        /* Query Mouse Pointer, Window Dimensions & Left Click state */
        Window root_return, child_return;
        int root_x, root_y;
        int win_x, win_y;
        unsigned int mask_return;
        XWindowAttributes wa;

        if (XQueryPointer(display, window, &root_return, &child_return,
                          &root_x, &root_y, &win_x, &win_y, &mask_return) &&
            XGetWindowAttributes(display, window, &wa))
        {
            int win_w = wa.width;
            int win_h = wa.height;

            if (win_w > 0 && win_h > 0)
            {
                /* 1. Mirror the aspect ratio math from render_present */
                const float TARGET_ASPECT = 640.0f / 360.0f;
                float win_aspect = (float)win_w / (float)win_h;

                int vp_x = 0, vp_y = 0;
                int vp_w = win_w, vp_h = win_h;

                if (win_aspect > TARGET_ASPECT)
                {
                    /* Pillarboxed (black bars on left/right) */
                    vp_w = (int)(win_h * TARGET_ASPECT);
                    vp_x = (win_w - vp_w) / 2;
                }
                else
                {
                    /* Letterboxed (black bars on top/bottom) */
                    vp_h = (int)(win_w / TARGET_ASPECT);
                    vp_y = (win_h - vp_h) / 2;
                }

                /* 2. Subtract offset and scale relative to viewport size */
                float norm_x = (float)(win_x - vp_x) / (float)vp_w;
                float norm_y = (float)(win_y - vp_y) / (float)vp_h;

                int32_t mouse_x = (int32_t)(norm_x * 640.0f);
                int32_t mouse_y = (int32_t)(norm_y * 360.0f);

                /* 3. Clamp so clicking in black bars doesn't go out of bounds */
                if (mouse_x < 0) mouse_x = 0;
                if (mouse_x >= 640) mouse_x = 639;

                if (mouse_y < 0) mouse_y = 0;
                if (mouse_y >= 360) mouse_y = 359;

                input_state.mouse_x = mouse_x;
                input_state.mouse_y = mouse_y;
            }
            else
            {
                input_state.mouse_x = (int32_t)win_x;
                input_state.mouse_y = (int32_t)win_y;
            }

            /* Button1Mask corresponds to Left Mouse Button */
            input_state.mouse_down = (mask_return & Button1Mask) != 0;
        }
    }

    /* 3. Update Joystick */
    if (joystick_fd >= 0)
    {
        struct js_event js;
        while (read(joystick_fd, &js, sizeof(js)) > 0)
        {
            if (js.type & JS_EVENT_BUTTON)
            {
                if (js.number < BUTTON_COUNT)
                    input_state.button_down[js.number] = (js.value != 0);
            }
        }
    }
}

/* Button API */
bool input_get_button(InputKey key)   { return input_state.button_down[key]; }
bool input_button_down(InputKey key) { return input_state.button_down[key] && !input_state.button_prev[key]; }
bool input_button_up(InputKey key)   { return !input_state.button_down[key] && input_state.button_prev[key]; }

/* Mouse API */
int32_t input_get_mouse_x(void)      { return input_state.mouse_x; }
int32_t input_get_mouse_y(void)      { return input_state.mouse_y; }

bool input_get_mouse_button(void)    { return input_state.mouse_down; }
bool input_mouse_button_down(void)   { return input_state.mouse_down && !input_state.mouse_prev; }
bool input_mouse_button_up(void)     { return !input_state.mouse_down && input_state.mouse_prev; }

#endif