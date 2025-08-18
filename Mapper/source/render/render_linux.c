#ifdef __linux__

#include "render.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static Display* display = NULL;
static Window window;
static GC gc;
static XImage* ximage = NULL;

static int fb_width = FB_WIDTH;
static int fb_height = FB_HEIGHT;

uint32_t* framebuffer = NULL;
uint32_t* framebuffer_mapview = NULL;
uint32_t* framebuffer_ui = NULL;

static void render_resize(int width, int height)
{
	fb_width = width;
	fb_height = height;

	if (framebuffer) free(framebuffer);
	if (framebuffer_mapview) free(framebuffer_mapview);
	if (framebuffer_ui) free(framebuffer_ui);

	framebuffer = calloc(fb_width * fb_height, sizeof(uint32_t));
	framebuffer_mapview = calloc(fb_width * fb_height, sizeof(uint32_t));
	framebuffer_ui = calloc(fb_width * fb_height, sizeof(uint32_t));

	if (ximage)
	{
		ximage->data = NULL; // prevent XDestroyImage from freeing framebuffer
		XDestroyImage(ximage);
		ximage = NULL;
	}

	ximage = XCreateImage(
		display, DefaultVisual(display, DefaultScreen(display)),
		24, ZPixmap, 0, (char*)framebuffer,
		fb_width, fb_height, 32, 0
	);
	// Make sure XDestroyImage doesn't free our framebuffer
	ximage->f.destroy_image = NULL;
}

void render_init(void* platform_context)
{
	// platform_context is expected to contain display + window
	Display* d = ((Display**)platform_context)[0];
	Window    w = *((Window*)((Display**)platform_context + 1));

	display = d;
	window = w;
	gc = DefaultGC(display, DefaultScreen(display));

	XWindowAttributes attrs;
	XGetWindowAttributes(display, window, &attrs);

	render_resize(attrs.width, attrs.height);
}

uint32_t* render_get_framebuffer(void)
{
	return framebuffer;
}

void render_clear(uint32_t* buffer, uint32_t color)
{
	if (!buffer) return;
	for (int i = 0; i < fb_width * fb_height; ++i)
		buffer[i] = color;
}

void render_blend_ui_over_game(void)
{
	for (int i = 0; i < fb_width * fb_height; ++i)
	{
		uint32_t ui_pixel = framebuffer_ui[i];
		uint8_t ui_alpha = (ui_pixel >> 24) & 0xFF;

		if (ui_alpha == 0)
		{
			framebuffer[i] = framebuffer_mapview[i];
		}
		else if (ui_alpha == 255)
		{
			framebuffer[i] = ui_pixel;
		}
		else
		{
			uint32_t game_pixel = framebuffer_mapview[i];
			uint8_t r_ui = (ui_pixel >> 16) & 0xFF;
			uint8_t g_ui = (ui_pixel >> 8) & 0xFF;
			uint8_t b_ui = ui_pixel & 0xFF;
			uint8_t r_game = (game_pixel >> 16) & 0xFF;
			uint8_t g_game = (game_pixel >> 8) & 0xFF;
			uint8_t b_game = game_pixel & 0xFF;
			uint8_t r_blend = (r_ui * ui_alpha + r_game * (255 - ui_alpha)) / 255;
			uint8_t g_blend = (g_ui * ui_alpha + g_game * (255 - ui_alpha)) / 255;
			uint8_t b_blend = (b_ui * ui_alpha + b_game * (255 - ui_alpha)) / 255;
			framebuffer[i] = (0xFF << 24) | (r_blend << 16) | (g_blend << 8) | b_blend;
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

	if (window_width != fb_width || window_height != fb_height)
	{
		render_resize(window_width, window_height);
	}

	XPutImage(display, window, gc, ximage,
		0, 0, 0, 0, fb_width, fb_height);

	XFlush(display);
}

void render_shutdown(void)
{
	if (ximage)
	{
		ximage->data = NULL;
		XDestroyImage(ximage);
		ximage = NULL;
	}
}

#endif // __linux__
