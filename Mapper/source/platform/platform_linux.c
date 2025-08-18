#ifdef __linux__

#include "platform.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>

static bool running = false;
static Display* display;
static Window window;
static GC gc;
X11Context x11;
static struct timespec last_time;
static Atom wm_delete_window;

void platform_init(const PlatformWindowDesc* desc)
{
	display = XOpenDisplay(NULL);
	if (!display)
	{
		fprintf(stderr, "XOpenDisplay fialed\n");
		return;
	}

	int screen = DefaultScreen(display);
	window = XCreateSimpleWindow(
		display, RootWindow(display, screen),
		10, 10, desc->width, desc->height,
		1, BlackPixel(display, screen),
		WhitePixel(display, screen));

	XStoreName(display, window, desc->title);
	XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);

	wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wm_delete_window, 1);

	gc = XCreateGC(display, window, 0, NULL);

	XMapWindow(display, window);
	XFlush(display);

	x11.display = display;
	x11.window = window;
	x11.gc = gc;

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

	XEvent event;
	while (XPending(display))
	{
		XNextEvent(display, &event);

		if (event.type == ClientMessage)
		{
			if ((Atom)event.xclient.data.l[0] == wm_delete_window)
				running = false;
		}
		else if (event.type == KeyPress)
		{
			KeySym key = XLookupKeysym(&event.xkey, 0);
			if (key == XK_Escape)
				running = false;
		}
	}
}

bool platform_running(void)
{
	return running;
}

void* platform_get_native_window(void)
{
	return (void*)&x11;
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
