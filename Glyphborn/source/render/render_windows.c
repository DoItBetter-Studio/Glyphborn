#ifdef _WIN32

#include "render.h"
#include <Windows.h>

static BITMAPINFO bmi;
static HDC hdcWindow;

uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_game[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_ui[FB_WIDTH * FB_HEIGHT];

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
		MessageBox(NULL, L"Failed to create framebuffer.", L"Error", MB_OK | MB_ICONERROR);
		return;
	}
	else
	{
		SelectObject(hdcWindow, hBitmap);
	}
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
		for (int i = 0; i < FB_WIDTH * FB_HEIGHT; ++i)
		{
			fb[i] = color;
		}
	}
}

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

void render_present(void)
{
	RECT rect;
	GetClientRect(WindowFromDC(hdcWindow), &rect);
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

void render_shutdown(void)
{
	if (hdcWindow)
	{
		ReleaseDC(WindowFromDC(hdcWindow), hdcWindow);
		hdcWindow = NULL;
	}
}

// 3D rendering functions (stub implementations)
static void render_draw_pixel(int x, int y, uint32_t color)
{
	if (x >= 0 && x < FB_WIDTH && y >= 0 && y < FB_HEIGHT)
	{
		framebuffer_game[y * FB_WIDTH + x] = color;
	}
}

static void render_draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
	int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;
	while (true)
	{
		render_draw_pixel(x0, y0, color);
		if (x0 == x1 && y0 == y1) break;
		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
}

void render_draw_wireframe(RenderMesh* mesh, Mat4 model, Mat4 view, Mat4 projection, uint32_t color)
{
	Vec3 projected[MAX_VERTICES];

	for (int i = 0; i < mesh->vertex_count; i++)
	{
		Vec4 v = { mesh->vertices[i].x, mesh->vertices[i].y, mesh->vertices[i].z, 1.0f };
		v = mat4_mul_vec4(model, v);
		v = mat4_mul_vec4(view, v);
		v = mat4_mul_vec4(projection, v);

		v.x /= v.w;
		v.y /= v.w;
		
		projected[i].x = (v.x + 1.0f) * 0.5f * FB_WIDTH;
		projected[i].y = (1.0f - (v.y + 1.0f) * 0.5f) * FB_HEIGHT; // Invert Y for top-down
	}

	for (int i = 0; i < mesh->edge_count; i++)
	{
		int v0 = mesh->edges[i][0];
		int v1 = mesh->edges[i][1];
		render_draw_line((int)projected[v0].x, (int)projected[v0].y,
						 (int)projected[v1].x, (int)projected[v1].y, color);
	}
}

#endif // _WIN32
