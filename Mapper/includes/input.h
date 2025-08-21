#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
	MOUSE_LEFT,
	MOUSE_MIDDLE,
	MOUSE_RIGHT,
	MOUSE_COUNT
} MouseButton;

typedef struct
{
	int keyboard_key;
} InputMapping;

typedef struct
{
	int mouse_x;
	int mouse_y;
	bool mouse_down[MOUSE_COUNT];
	bool mouse_prev[MOUSE_COUNT];
} InputState;

void input_init(void);
void input_update(void);

int input_get_mouse_x(void);
int input_get_mouse_y(void);
bool input_get_mouse(MouseButton button);
bool input_get_mouse_down(MouseButton button);
bool input_get_mouse_up(MouseButton button);

#endif // !INPUT_H
