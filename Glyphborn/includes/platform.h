#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

#define TARGET_FPS			1000
#define TARGET_FRAME_TIME	(1000 / TARGET_FPS)

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
