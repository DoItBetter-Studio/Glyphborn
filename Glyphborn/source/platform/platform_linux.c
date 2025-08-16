#ifdef __linux__

#include "platform.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

static bool running = false;
static Display* display;
static Window window;

static struct timespec last_time;

void platform_init(const PlatformWindowDesc* desc)
{
	display = XOpenDisplay(NULL);
	if (!display) return;

	int screen = DefaultScreen(display);
	window = XCreateSimpleWindow(
		display, RootWindow(display, screen),
		10, 10, desc->width, desc->height,
		1, BlackPixel(display, screen),
		WhitePixel(display, screen));

	XStoreName(display, window, desc->title);
	XSelectInput(display, window, ExposureMask | KeyPress | KeyRelease | StructureNotifyMask);
	XMapWindow(display, window);

	running = true;
	clock_gettime(CLOCK_MONOTONIC, &last_time);
}

void platform_shutdown(void)
{
	if (display)
	{
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = NULL;
	}
	running = false;
}

void platform_poll_events(void)
{
	if (!display) return;

	while (XPending(display))
	{
		XEvent event;
		XNextEvent(display, &event);

		if (event.type == DestroyNotify) running = false;
	}
}

bool platform_running(void)
{
	return running;
}

void* platform_get_native_window(void)
{
	return (void*)window;
}

float platform_frame_timing(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	double elapsed = (now.tv_sec - last_time.tv_sec) + (now.tv_nsec - last_time.tv_nsec) / 1e9;
	double frame_target = 1.0 / TARGET_FPS;

	if (elapsed < frame_target)
	{
		usleep((useconds_t)((frame_target - elapsed) * 1e6)); // Sleep for the remaining time
		clock_gettime(CLOCK_MONOTONIC, &now);
		elapsed = (now.tv_sec - last_time.tv_sec) + (now.tv_nsec - last_time.tv_nsec) / 1e9;
	}

	last_time = now;
	return (float)elapsed;
}
#endif // __linux__
