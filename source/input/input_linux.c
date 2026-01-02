#ifdef __linux__
#include "input.h"
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

static InputMapping button_mappings[BUTTON_COUNT] = {
	[BUTTON_UP] = { 'W', 0x01 },
	[BUTTON_DOWN] = { 'S', 0x02 },
	[BUTTON_LEFT] = { 'A', 0x04 },
	[BUTTON_RIGHT] = { 'D', 0x08 },
	[BUTTON_A] = { 'Z', 0x10 },
	[BUTTON_B] = { 'X', 0x20 },
	[BUTTON_X] = { 'Q', 0x40 },
	[BUTTON_Y] = { 'E', 0x80 },
	[BUTTON_LEFT_BUMPER] = { 'L', 0x100 },
	[BUTTON_RIGHT_BUMPER] = { 'R', 0x200 },
	[BUTTON_START] = { '\r', 0x400 },
	[BUTTON_SELECT] = { '\t', 0x800 },
};

static InputState input_state;
static Display* display = NULL;
static Window root_window;
static int joystick_fd = -1;

void input_init(void)
{
	memset(&input_state, 0, sizeof(InputState));

	display = XOpenDisplay(NULL);
	if (display) root_window = DefaultRootWindow(display);

	joystick_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK);
	if (joystick_fd < 0)
	{
		printf("Failed to open joystick device: %s\n", strerror(errno));
		joystick_fd = -1;
	}
}

void input_update(void)
{
	for (int i = 0; i < BUTTON_COUNT; ++i)
		input_state.button_prev[i] = input_state.button_down[i];

	if (display)
	{
		char keys[32];
		XQueryKeymap(display, keys);

		for (int i = 0; i < BUTTON_COUNT; ++i)
		{
			KeySym keysym = button_mappings[i].keyboard_key;
			KeyCode keycode = XKeysymToKeycode(display, keysym);
			bool key_down = (keys[keycode / 8] & (1 << (keycode % 8))) != 0;

			input_state.button_down[i] = key_down;
		}
	}

	if (joystick_fd >= 0)
	{
		struct js_event js;
		while (read(joystick_fd, &js, sizeof(js)) > 0)
		{
			if (js.type & JS_EVENT_BUTTON)
			{
				for (int i = 0; i < BUTTON_COUNT; i++)
				{
					if (js.number == i)
						input_state.button_down[i] |= js.value;
				}
			}
		}
	}
}

bool input_get_button(InputKey key)
{
	return input_state.button_down[key];
}

bool input_button_down(InputKey key)
{
	return input_state.button_down[key] && !input_state.button_prev[key];
}

bool input_button_up(InputKey key)
{
	return !input_state.button_down[key] && input_state.button_prev[key];
}
#endif