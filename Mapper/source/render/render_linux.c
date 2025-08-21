#ifdef __linux__

#include "render.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

static Display* display = NULL;
static Window window;
static GC gc;
static XImage* ximage = NULL;

int fb_width;
int fb_height;

uint32_t* framebuffer = NULL;
uint32_t* framebuffer_mapview = NULL;
uint32_t* framebuffer_ui = NULL;
float* depthbuffer_mapview = NULL;

static void render_resize(int width, int height)
{
	fb_width = width;
	fb_height = height;

	if (ximage)
	{
		ximage->data = NULL; // prevent XDestroyImage from freeing framebuffer
		XDestroyImage(ximage);
	}

	if (framebuffer) free(framebuffer);
	if (framebuffer_mapview) free(framebuffer_mapview);
	if (framebuffer_ui) free(framebuffer_ui);

	framebuffer = calloc(fb_width * fb_height, sizeof(uint32_t));
	framebuffer_mapview = calloc(fb_width * fb_height, sizeof(uint32_t));
	framebuffer_ui = calloc(fb_width * fb_height, sizeof(uint32_t));

	ximage = XCreateImage(
		display, DefaultVisual(display, DefaultScreen(display)),
		24, ZPixmap, 0, (char*)framebuffer,
		fb_width, fb_height, 32, 0
	);
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
		0, 0, 0, 0, window_width, window_height);

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
