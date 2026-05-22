#ifdef __YGGDRASIL__
#include "input.h"
#include <yggdrasil.h>
#include <string.h>

/* Maps ps2 bitmask bit positions to InputKey enum order */
static const uint16_t button_masks[BUTTON_COUNT] = {
    [BUTTON_UP]           = YGG_BTN_UP,
    [BUTTON_DOWN]         = YGG_BTN_DOWN,
    [BUTTON_LEFT]         = YGG_BTN_LEFT,
    [BUTTON_RIGHT]        = YGG_BTN_RIGHT,
    [BUTTON_A]            = YGG_BTN_A,
    [BUTTON_B]            = YGG_BTN_B,
    [BUTTON_X]            = YGG_BTN_X,
    [BUTTON_Y]            = YGG_BTN_Y,
    [BUTTON_RIGHT_BUMPER] = YGG_BTN_RIGHT_BUMPER,
    [BUTTON_LEFT_BUMPER]  = YGG_BTN_LEFT_BUMPER,
    [BUTTON_START]        = YGG_BTN_START,
    [BUTTON_SELECT]       = YGG_BTN_SELECT,
};

static InputState input_state;

void input_init(void) {
    memset(&input_state, 0, sizeof(InputState));
}

void input_update(void) {
    ps2_update();
    uint16_t btns = ps2_get_buttons();

    for (int i = 0; i < BUTTON_COUNT; i++) {
        input_state.button_prev[i] = input_state.button_down[i];
        input_state.button_down[i] = (btns & button_masks[i]) != 0;
    }
}

bool input_get_button(InputKey key) {
    return input_state.button_down[key];
}

bool input_button_down(InputKey key) {
    return input_state.button_down[key] && !input_state.button_prev[key];
}

bool input_button_up(InputKey key) {
    return !input_state.button_down[key] && input_state.button_prev[key];
}
#endif /* __YGGDRASIL__ */