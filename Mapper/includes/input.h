#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_CONTROLLERS		1
#define KEY_COUNT			256

typedef enum
{
	BUTTON_UP,
	BUTTON_DOWN,
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_A,
	BUTTON_B,
	BUTTON_X,
	BUTTON_Y,
	BUTTON_RIGHT_BUMPER,
	BUTTON_LEFT_BUMPER,
	BUTTON_START,
	BUTTON_SELECT,
	BUTTON_COUNT
} InputKey;

typedef struct
{
	int keyboard_key;
	uint16_t controller_button;
} InputMapping;

typedef struct
{
	bool button_down[BUTTON_COUNT];
	bool button_prev[BUTTON_COUNT];

	int mouse_x;
	int mouse_y;
	bool mouse_left;
	bool mouse_right;
	bool mouse_middle;
} InputState;

void input_init(void);
void input_update(void);
bool input_get_button(InputKey key);
bool input_button_down(InputKey key);
bool input_button_up(InputKey key);

int input_get_mouse_x(void);
int input_get_mouse_y(void);
bool input_get_mouse_left(void);
bool input_get_mouse_right(void);
bool input_get_mouse_middle(void);

#endif // !INPUT_H
