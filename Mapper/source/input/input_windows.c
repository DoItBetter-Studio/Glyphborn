#ifdef _WIN32

#include "input.h"
#include <Windows.h>
#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

static InputMapping button_mappings[BUTTON_COUNT] = {
	[BUTTON_UP]				= { 'W', XINPUT_GAMEPAD_DPAD_UP },
	[BUTTON_DOWN]			= { 'S', XINPUT_GAMEPAD_DPAD_DOWN },
	[BUTTON_LEFT]			= { 'A', XINPUT_GAMEPAD_DPAD_LEFT },
	[BUTTON_RIGHT]			= { 'D', XINPUT_GAMEPAD_DPAD_RIGHT },
	[BUTTON_A]				= { 'Z', XINPUT_GAMEPAD_A },
	[BUTTON_B]				= { 'X', XINPUT_GAMEPAD_B },
	[BUTTON_X]				= { 'Q', XINPUT_GAMEPAD_X },
	[BUTTON_Y]				= { 'E', XINPUT_GAMEPAD_Y },
	[BUTTON_LEFT_BUMPER]	= { VK_SHIFT, XINPUT_GAMEPAD_LEFT_SHOULDER },
	[BUTTON_RIGHT_BUMPER]	= { VK_CONTROL, XINPUT_GAMEPAD_RIGHT_SHOULDER },
	[BUTTON_START]			= { VK_RETURN, XINPUT_GAMEPAD_START },
	[BUTTON_SELECT]			= { VK_TAB, XINPUT_GAMEPAD_BACK },
};

static InputState input_state;

void input_init(void)
{
	ZeroMemory(&input_state, sizeof(InputState));
}

void input_update(void)
{
	for (int i = 0; i < BUTTON_COUNT; ++i)
		input_state.button_prev[i] = input_state.button_down[i];

	XINPUT_STATE state;
	DWORD result = XInputGetState(0, &state);
	bool controller_connected = (result == ERROR_SUCCESS);

	for (int i = 0; i < BUTTON_COUNT; i++)
	{
		const InputMapping* map = &button_mappings[i];
		bool key_down = (GetAsyncKeyState(map->keyboard_key) & 0x8000) != 0;
		bool pad_down = controller_connected && (state.Gamepad.wButtons & map->controller_button);

		input_state.button_down[i] = key_down || pad_down;
	}

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(GetActiveWindow(), &pt);
	input_state.mouse_x = pt.x;
	input_state.mouse_y = pt.y;

	input_state.mouse_left = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	input_state.mouse_right = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	input_state.mouse_middle = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
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

int input_get_mouse_x(void)
{
	return input_state.mouse_x;
}

int input_get_mouse_y(void)
{
	return input_state.mouse_y;
}

bool input_get_mouse_left(void)
{
	return input_state.mouse_left;
}

bool input_get_mouse_right(void)
{
	return input_state.mouse_right;
}

bool input_get_mouse_middle(void)
{
	return input_state.mouse_middle;
}

#endif // _WIN32
