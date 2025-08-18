#ifdef __linux__

#include "render.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <linux/types.h>

typedef struct
{
	Display* display;
	Window window;
	GC gc;
} X11Context;

static Display* display = NULL;
static Window window;
static GC gc;
static XImage* ximage = NULL;

uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_game[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_ui[FB_WIDTH * FB_HEIGHT];

void render_init(void* platform_context)
{
	X11Context* context = (X11Context*)platform_context;
	window = context->window;
	display = context->display;
	gc = context->gc ? context->gc : DefaultGC(display, DefaultScreen(display));

	ximage = XCreateImage(
		display, DefaultVisual(display, DefaultScreen(display)),
		24, ZPixmap, 0, (char*)framebuffer,
		FB_WIDTH, FB_HEIGHT, 32, 0
	);

	ximage->f.destroy_image = NULL;
}

uint32_t* render_get_framebuffer(void)
{
	return framebuffer;
}

void render_clear(uint32_t* buffer, uint32_t color)
{
	for (int i = 0; i < FB_WIDTH * FB_HEIGHT; ++i)
	{
		buffer[i] = color;
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
	if (!display || !ximage) return;

	XWindowAttributes attrs;
	XGetWindowAttributes(display, window, &attrs);
	int window_width = attrs.width;
	int window_height = attrs.height;

	float scale_x = (float)window_width / FB_WIDTH;
	float scale_y = (float)window_height / FB_HEIGHT;
	float scale = (scale_x < scale_y) ? scale_x : scale_y;

	int render_width = (int)(FB_WIDTH * scale);
	int render_height = (int)(FB_HEIGHT * scale);

	int offset_x = (window_width - render_width) / 2;
	int offset_y = (window_height - render_height) / 2;

	XSetForeground(display, gc, 0x000000);
	if (offset_y > 0)
	{
		XFillRectangle(display, window, gc, 0, 0, window_width, offset_y);
		XFillRectangle(display, window, gc, 0, offset_y + render_height, window_width, offset_y);
	}
	if (offset_x > 0)
	{
		XFillRectangle(display, window, gc, 0, 0, offset_x, window_height);
		XFillRectangle(display, window, gc, offset_x + render_width, 0, window_width, window_height);
	}

	XImage* scaled_image = XCreateImage(
		display, DefaultVisual(display, DefaultScreen(display)),
		24, ZPixmap, 0, (char*)malloc(render_width * render_height * 4),
		render_width, render_height, 32, 0
	);

	for (int y = 0; y < render_height; y++)
	{
		int src_y = y / scale;
		if (src_y >= FB_HEIGHT) src_y = FB_HEIGHT - 1;

		for (int x = 0; x < render_width; x++)
		{
			int src_x = x / scale;
			if (src_x >= FB_WIDTH) src_x = FB_WIDTH - 1;

			uint32_t color = framebuffer[src_y * FB_WIDTH + src_x];
			XPutPixel(scaled_image, x, y, color);
		}
	}


	XPutImage(display, window, gc, scaled_image,
		0, 0, offset_x, offset_y, render_width, render_height);

	XFlush(display);
	XDestroyImage(scaled_image);
}


void render_shutdown(void)
{
	if (ximage)
	{
		ximage->data = NULL;
		free(ximage);
		ximage = NULL;
	}
}

#endif // __linux__
