#ifdef __linux__

#include "render.h"
#include <X11/Xlib.h>
#include <string.h>
#include <linux/types.h>

static Display* display = NULL;
static Window window;
static GC gc;
static XImage* ximage = NULL;

void render_init(void* platform_context)
{
	window = (Window)platform_context;
	display = XOpenDisplay(NULL);
	if (!display) return;

	gc = DefaultGC(display, DefaultScreen(display));

	ximage = XCreateImage(
		display, DefaultVisual(display, DefaultScreen(display)),
		24, ZPixmap, 0, (char*)framebuffer,
		FB_WIDTH, FB_HEIGHT, 32, 0
	);
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

	XPutImage(display, window, gc, ximage,
		0, 0, 0, 0, FB_WIDTH, FB_HEIGHT);
	
	XFlush(display);
}


void render_shutdown(void)
{
	if (ximage)
	{
		XDestroyImage(ximage);
		ximage = NULL;
	}
	if (display)
	{
		XCloseDisplay(display);
		display = NULL;
	}
}
#endif // __linux__
