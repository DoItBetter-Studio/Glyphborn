#ifdef _WIN32

#include "render.h"
#include <windows.h>

/* Win32 renderer implementation: provides framebuffer storage and presents via GDI */
static BITMAPINFO bmi;
static HDC hdcWindow;

/* Actual framebuffer storage (defined here for the platform) */
uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_game[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_ui[FB_WIDTH * FB_HEIGHT];

/* Depth buffer used by the rasterizer (defined here) */
float depthbuffer[FB_WIDTH * FB_HEIGHT]; // <-- actual definition lives here

/**
 * render_init - Initialize the Win32 rendering backend and create a DIB section
 * @platform_context: HWND pointer cast to void*
 */
void render_init(void* platform_context)
{
	HWND hwnd = (HWND)platform_context;
	hdcWindow = GetDC(hwnd);

	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = FB_WIDTH;
	bmi.bmiHeader.biHeight = -FB_HEIGHT; // Negative height for top-down bitmap
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBitmap = CreateDIBSection(hdcWindow, &bmi, DIB_RGB_COLORS, (void**) & framebuffer, NULL, 0);
	if (!hBitmap)
	{
		MessageBox(NULL, "Failed to create framebuffer.", "Error", MB_OK | MB_ICONERROR);
		return;
	}
	else
	{
		SelectObject(hdcWindow, hBitmap);
	}
}

/**
 * render_get_framebuffer - Return pointer to platform framebuffer storage
 */
uint32_t* render_get_framebuffer(void)
{
	return framebuffer;
}

/**
 * render_clear - Fill a buffer with a color
 */
void render_clear(uint32_t* buffer, uint32_t color)
{
	if (buffer)
	{
		uint32_t* fb = (uint32_t*)buffer;
		for (int i = 0; i < FB_WIDTH * FB_HEIGHT; ++i)
		{
			fb[i] = color;
		}
	}
}

/**
 * render_blend_ui_over_game - Blend UI layer over the game layer
 */
void render_blend_ui_over_game()
{
	for (int i = 0; i < FB_WIDTH * FB_HEIGHT; ++i)
	{
		uint32_t ui_pixel = framebuffer_ui[i];
		uint8_t ui_alpha = (ui_pixel >> 24) & 0xFF;

		if (ui_alpha == 0)
		{
			framebuffer[i] = framebuffer_game[i];
		}
		else if (ui_alpha == 255)
		{
			framebuffer[i] = ui_pixel;
		}
		else
		{
			uint32_t game_pixel = framebuffer_game[i];

			uint8_t r_ui = (ui_pixel >> 16) & 0xFF;
			uint8_t g_ui = (ui_pixel >> 8) & 0xFF;
			uint8_t b_ui = ui_pixel & 0xFF;

			uint8_t r_game = (game_pixel >> 16) & 0xFF;
			uint8_t g_game = (game_pixel >> 8) & 0xFF;
			uint8_t b_game = game_pixel & 0xFF;

			uint8_t r_blend = (r_ui * ui_alpha + r_game * (255 - ui_alpha)) / 255;
			uint8_t g_blend = (g_ui * ui_alpha + g_game * (255 - ui_alpha)) / 255;
			uint8_t b_blend = (b_ui * ui_alpha + b_game * (255 - ui_alpha)) / 255;

			framebuffer[i] = (ui_alpha << 24) | (r_blend << 16) | (g_blend << 8) | b_blend;
		}
	}
}

/**
 * render_present - Present framebuffer to the window, performing scaling and letterbox bars.
 */
void render_present(void)
{
	RECT rect;
	HWND hwnd = WindowFromDC(hdcWindow);
	GetClientRect(hwnd, &rect);

	int window_width = rect.right - rect.left;
	int window_height = rect.bottom - rect.top;

	float scale_x = (float)window_width / FB_WIDTH;
	float scale_y = (float)window_height / FB_HEIGHT;
	float scale = (scale_x < scale_y) ? scale_x : scale_y;

	int render_width = (int)(FB_WIDTH * scale);
	int render_height = (int)(FB_HEIGHT * scale);

	int offset_x = (window_width - render_width) / 2;
	int offset_y = (window_height - render_height) / 2;

	if (offset_x > 0)
	{
		RECT bar = { 0, 0, offset_x, window_height };
		FillRect(hdcWindow, &bar, (HBRUSH)GetStockObject(BLACK_BRUSH));

		bar.left = offset_x + render_width;
		bar.right = window_width;
		FillRect(hdcWindow, &bar, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}

	if (offset_y > 0)
	{
		RECT bar = { 0, 0, window_width, offset_y };
		FillRect(hdcWindow, &bar, (HBRUSH)GetStockObject(BLACK_BRUSH));

		bar.top = offset_y + render_height;
		bar.bottom = window_height;
		FillRect(hdcWindow, &bar, (HBRUSH)GetStockObject(BLACK_BRUSH));
	}

	StretchDIBits(
		hdcWindow,
		offset_x, offset_y, render_width, render_height,
		0, 0, FB_WIDTH, FB_HEIGHT,
		framebuffer,
		&bmi,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

/**
 * render_shutdown - Release platform resources
 */
void render_shutdown(void)
{
	if (hdcWindow)
	{
		ReleaseDC(WindowFromDC(hdcWindow), hdcWindow);
		hdcWindow = NULL;
	}
}

#endif // _WIN32
