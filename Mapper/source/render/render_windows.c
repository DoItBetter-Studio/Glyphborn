#ifdef _WIN32

#include "render.h"
#include <Windows.h>

static BITMAPINFO bmi;
static HDC hdcWindow;
static HBITMAP hBitmap = NULL;

uint32_t* framebuffer = NULL;
uint32_t* framebuffer_mapview = NULL;
uint32_t* framebuffer_ui = NULL;

float* depthbuffer_mapview = NULL;

static void render_resize(int width, int height)
{
	fb_width = width;
	fb_height = height;

	if (framebuffer_ui) free(framebuffer_ui);

	framebuffer_ui = calloc(fb_width * fb_height, sizeof(uint32_t));

	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = fb_width;
	bmi.bmiHeader.biHeight = -fb_height; // Negative height for top-down bitmap
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	if (hBitmap)
	{
		DeleteObject(hBitmap);
		hBitmap = NULL;
	}

	hBitmap = CreateDIBSection(hdcWindow, &bmi, DIB_RGB_COLORS, (void**)&framebuffer, NULL, 0);
	if (!hBitmap)
		return;

	SelectObject(hdcWindow, hBitmap);
}

void render_init(void* platform_context)
{
	HWND hwnd = (HWND)platform_context;
	hdcWindow = GetDC(hwnd);

	RECT rect;
	GetClientRect(WindowFromDC(hdcWindow), &rect);
	int window_width = rect.right - rect.left;
	int window_height = rect.bottom - rect.top;

	framebuffer = calloc(window_width * window_height, sizeof(uint32_t));
	render_resize(window_width, window_height);
}

uint32_t* render_get_framebuffer(void)
{
	return framebuffer;
}

void render_clear(uint32_t* buffer, uint32_t color)
{
	if (buffer)
	{
		uint32_t* fb = (uint32_t*)buffer;
		for (int i = 0; i < fb_width * fb_height; ++i)
		{
			fb[i] = color;
		}
	}
}

void render_blend_ui_under_mapview()
{
	for (int i = 0; i < fb_width * fb_height; ++i)
	{
		uint32_t base = framebuffer[i];               // Already contains map view
		uint32_t ui_pixel = framebuffer_ui[i];        // UI is the translucent background

		uint8_t alpha = (ui_pixel >> 24) & 0xFF;

		if (alpha == 0)
		{
			// UI is fully transparent, keep map view
			continue;
		}
		else if (alpha == 255)
		{
			// UI is fully opaque, overwrite
			framebuffer[i] = ui_pixel;
		}
		else
		{
			uint8_t r_ui = (ui_pixel >> 16) & 0xFF;
			uint8_t g_ui = (ui_pixel >> 8) & 0xFF;
			uint8_t b_ui = ui_pixel & 0xFF;

			uint8_t r_base = (base >> 16) & 0xFF;
			uint8_t g_base = (base >> 8) & 0xFF;
			uint8_t b_base = base & 0xFF;

			uint8_t r = (r_ui * alpha + r_base * (255 - alpha)) / 255;
			uint8_t g = (g_ui * alpha + g_base * (255 - alpha)) / 255;
			uint8_t b = (b_ui * alpha + b_base * (255 - alpha)) / 255;

			framebuffer[i] = (255 << 24) | (r << 16) | (g << 8) | b;
		}
	}
}

void render_present(void)
{
	RECT rect;
	GetClientRect(WindowFromDC(hdcWindow), &rect);
	int window_width = rect.right - rect.left;
	int window_height = rect.bottom - rect.top;

	if (window_width != fb_width || window_height != fb_height)
	{
		render_resize(window_width, window_height);
	}
	
	StretchDIBits(
		hdcWindow,
		0, 0, fb_width, fb_height,
		0, 0, fb_width, fb_height,
		framebuffer,
		&bmi,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

void render_shutdown(void)
{
	if (hBitmap)
	{
		DeleteObject(hBitmap);
		hBitmap = NULL;
	}

	if (hdcWindow)
	{
		ReleaseDC(WindowFromDC(hdcWindow), hdcWindow);
		hdcWindow = NULL;
	}
}

#endif // _WIN32
