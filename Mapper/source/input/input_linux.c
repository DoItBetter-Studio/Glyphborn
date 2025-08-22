#ifdef __linux__
#include "input.h"
#include "platform.h"
#include <X11/keysym.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static InputState input_state;
static Display* display = NULL;
static Window root_window;
static int joystick_fd = -1;

void input_init(void)
{
	memset(&input_state, 0, sizeof(InputState));

	// platform_context is expected to contain display + window
	display = x11.display;
	root_window = x11.window;

	joystick_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
	if (joystick_fd < 0)
	{
		printf("Failed to open joystick device: %s\n", strerror(errno));
		joystick_fd = -1;
	}
}

void input_update(void)
{
	if (display)
	{
		Window root_return, child_return;
		int root_x, root_y, win_x, win_y;
		unsigned int mask;

		if (XQueryPointer(display, root_window,	&root_return, &child_return, &root_x, &root_y, &win_x, &win_y, &mask))
		{
			for (int i = 0; i < MOUSE_COUNT; ++i)
				input_state.mouse_prev[i] = input_state.right_mouse_down[i];

			input_state.mouse_x = win_x;
			input_state.mouse_y = win_y;

			input_state.right_mouse_down[0] = (mask & Button1Mask) != 0;
			input_state.right_mouse_down[1] = (mask & Button2Mask) != 0;
			input_state.right_mouse_down[2] = (mask & Button3Mask) != 0;
		}
	}
}

int input_get_mouse_x(void)
{
	return input_state.mouse_x;
}

int input_get_mouse_y(void)
{
	return input_state.mouse_y;
}

bool input_get_mouse(MouseButton button)
{
	return input_state.right_mouse_down[button];
}

bool input_get_mouse_down(MouseButton button)
{
	return input_state.right_mouse_down[button] && !input_state.mouse_prev[button];
}

bool input_get_mouse_up(MouseButton button)
{
	return !input_state.right_mouse_down[button] && input_state.mouse_prev[button];
}

#endif