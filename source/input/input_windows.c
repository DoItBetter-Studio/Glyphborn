#ifdef _WIN32

#include "input/input.h"
#include <windows.h>
#include <xinput.h>

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
static HWND input_window;

void input_init(void* platform_context)
{
	ZeroMemory(&input_state, sizeof(InputState));
	input_window = (HWND)platform_context;
}

void input_update(void)
{
	for (int32_t i = 0; i < BUTTON_COUNT; ++i)
		input_state.button_prev[i] = input_state.button_down[i];

	XINPUT_STATE state;
	DWORD result = XInputGetState(0, &state);
	bool controller_connected = (result == ERROR_SUCCESS);

	for (int32_t i = 0; i < BUTTON_COUNT; i++)
	{
		const InputMapping* map = &button_mappings[i];
		bool key_down = (GetAsyncKeyState(map->keyboard_key) & 0x8000) != 0;
		bool pad_down = controller_connected && (state.Gamepad.wButtons & map->controller_button);

		input_state.button_down[i] = key_down || pad_down;
	}

	input_state.mouse_prev = input_state.mouse_down;

	POINT cursor;
    RECT client_rect;

    if (GetCursorPos(&cursor) && input_window && GetClientRect(input_window, &client_rect))
    {
        ScreenToClient(input_window, &cursor);

        int win_w = client_rect.right - client_rect.left;
        int win_h = client_rect.bottom - client_rect.top;

        if (win_w > 0 && win_h > 0)
        {
            /* 1. Calculate aspect ratio viewport bounds */
            const float TARGET_ASPECT = 640.0f / 360.0f;
            float win_aspect = (float)win_w / (float)win_h;

            int vp_x = 0, vp_y = 0;
            int vp_w = win_w, vp_h = win_h;

            if (win_aspect > TARGET_ASPECT)
            {
                /* Pillarboxed (bars on left/right) */
                vp_w = (int)(win_h * TARGET_ASPECT);
                vp_x = (win_w - vp_w) / 2;
            }
            else
            {
                /* Letterboxed (bars on top/bottom) */
                vp_h = (int)(win_w / TARGET_ASPECT);
                vp_y = (win_h - vp_h) / 2;
            }

            /* 2. Normalize relative to active viewport */
            float norm_x = (float)(cursor.x - vp_x) / (float)vp_w;
            float norm_y = (float)(cursor.y - vp_y) / (float)vp_h;

            int32_t mouse_x = (int32_t)(norm_x * 640.0f);
            int32_t mouse_y = (int32_t)(norm_y * 360.0f);

            /* 3. Clamp inside 640x360 bounds */
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_x >= 640) mouse_x = 639;

            if (mouse_y < 0) mouse_y = 0;
            if (mouse_y >= 360) mouse_y = 359;

            input_state.mouse_x = mouse_x;
            input_state.mouse_y = mouse_y;
        }
        else
        {
            input_state.mouse_x = cursor.x;
            input_state.mouse_y = cursor.y;
        }
    }

	input_state.mouse_down = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
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

int32_t input_get_mouse_x(void)
{
	return input_state.mouse_x;
}

int32_t input_get_mouse_y(void)
{
	return input_state.mouse_y;
}

bool input_get_mouse_button(void)
{
	return input_state.mouse_down;
}

bool input_mouse_button_down(void)
{
	return input_state.mouse_down && !input_state.mouse_prev;
}

bool input_mouse_button_up(void)
{
	return !input_state.mouse_down && input_state.mouse_prev;
}

#endif // _WIN32
