#ifdef _WIN32

#include "input.h"
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

static InputMapping button_mappings[MOUSE_COUNT] = {
	[MOUSE_LEFT]			= { VK_LBUTTON },
	[MOUSE_MIDDLE]			= { VK_MBUTTON },
	[MOUSE_RIGHT]			= { VK_RBUTTON },
};

static InputState input_state;

void input_init(void)
{
	ZeroMemory(&input_state, sizeof(InputState));
}

void input_update(void)
{
	for (int i = 0; i < MOUSE_COUNT; ++i)
		input_state.mouse_prev[i] = input_state.mouse_down[i];

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(GetActiveWindow(), &pt);
	input_state.mouse_x = pt.x;
	input_state.mouse_y = pt.y;

	input_state.mouse_down[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	input_state.mouse_down[1] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
	input_state.mouse_down[2] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
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
	return input_state.mouse_down[button];
}

bool input_get_mouse_down(MouseButton button)
{
	return input_state.mouse_down[button] && !input_state.mouse_prev[button];
}

bool input_get_mouse_up(MouseButton button)
{
	return !input_state.mouse_down[button] && input_state.mouse_prev[button];
}

#endif // _WIN32
