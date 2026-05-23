#ifdef __linux__

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <unistd.h>

#include "platform.h"
#include "version.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct
{
	Display* display;
	Window window;
	GC gc;
} X11Context;


static bool running = false;
static Display* display;
static Window window;
static GC gc;
static X11Context x11;
static struct timespec last_time;
static Atom wm_delete_window;

const uint8_t* g_asset_volumes[MAX_VOLUMES] = { 0 };
static int g_volume_fds[MAX_VOLUMES]        = { 0 };

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
	char title[128];
	snprintf(title, sizeof(title), "%s v%s", desc->title, gb_version);
	XStoreName(display, window, title);

	XSelectInput(display, window, ExposureMask | KeyPress | KeyRelease | StructureNotifyMask);

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

void platform_set_window_title(const char* title)
{
    if (display && window)
    {
        XStoreName(display, window, title);
        XFlush(display); // Force X11 to redraw the title bar immediately
    }
}

void platform_init_assets(int total_volumes)
{
	for (int i = 0; i < total_volumes && i < MAX_VOLUMES; i++)
	{
		char filename[64];
		snprintf(filename, sizeof(filename), "data/data_%03d.dat", i);

		int fd = open(filename, O_RDONLY);
		if (fd < 0)
		{
			continue;
		}
		g_volume_fds[i] = fd;

		// Hinting flags:
		// MAP_PRIVATE protects internal file alignment
		// PROT_READ mirrors your exact compilation .rodata goals
		void* mapped_address = mmap(NULL, VOLUME_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
		
		if (mapped_address == MAP_FAILED)
		{
			close(fd);
			g_volume_fds[i] = -1;
			continue;
		}

		// Advise the Linux kernel that we will access this sequentially 
		// to maximize hardware-level lookahead reading performance
		madvise(mapped_address, VOLUME_SIZE, MADV_SEQUENTIAL);

		g_asset_volumes[i] = (const uint8_t*)mapped_address;
	}
}

void platform_shutdown_assets(int total_volumes)
{
	for (int i = 0; i < total_volumes; i++)
	{
		if (g_asset_volumes[i] && g_asset_volumes[i] != MAP_FAILED)
		{
			munmap((void*)g_asset_volumes[i], VOLUME_SIZE);
			g_asset_volumes[i] = NULL;
		}
		if (g_volume_fds[i] >= 0)
		{
			close(g_volume_fds[i]);
			g_volume_fds[i] = -1;
		}
	}
}
#endif // __linux__
