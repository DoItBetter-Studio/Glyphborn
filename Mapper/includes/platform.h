#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

#define TARGET_FPS			60
#define TARGET_FRAME_TIME	(1000 / TARGET_FPS)

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
typedef struct
{
	Display* display;
	Window window;
	GC gc;
} X11Context;

extern X11Context x11;
#endif

typedef struct
{
	int width;
	int height;
	const char* title;
} PlatformWindowDesc;

void platform_init(const PlatformWindowDesc *desc);
void platform_shutdown(void);
void platform_poll_events(void);
bool platform_running(void);
float platform_frame_timing(void);
void* platform_get_native_window(void);

#endif // !PLATFORM_H
