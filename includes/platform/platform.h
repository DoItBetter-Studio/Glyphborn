#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#define TARGET_FPS			60.0f
#define TARGET_FRAME_TIME	(1000.0f / TARGET_FPS)
#define MAX_VOLUMES			32
#define VOLUME_SIZE			(4ULL * 1024ULL * 1024ULL * 1024ULL) // Fixed 4GB scale

typedef struct
{
	int32_t width;
	int32_t height;
	const char* title;
} PlatformWindowDesc;

extern const uint8_t* g_asset_volumes[MAX_VOLUMES];

void platform_init(const PlatformWindowDesc *desc);
void platform_shutdown(void);
void platform_poll_events(void);
bool platform_running(void);
float platform_frame_timing(void);
void* platform_get_native_window(void);
void platform_set_window_title(const char* title);
void platform_init_assets(int total_volumes);
void platform_shutdown_assets(int total_volumes);

static inline const uint8_t* platform_get_asset(uint64_t global_offset)
{
    uint64_t volume_index = global_offset / VOLUME_SIZE;
    uint64_t local_offset  = global_offset % VOLUME_SIZE;
    
    // Safety boundary protection against unmapped segments
    if (volume_index >= MAX_VOLUMES || !g_asset_volumes[volume_index])
    {
        printf("\n==================================================\n");
        printf("❌ ASSET ERROR: Volume %" PRIu64 " is not loaded!\n", volume_index);
        printf("   Attempted Global Offset: %" PRIu64 " (0x%" PRIX64 ")\n", global_offset, global_offset);
        printf("==================================================\n");
        printf("Press ENTER in this console to see crash stack trace...");
        
#ifdef _WIN32
        // Force the win32 console to halt so it won't instantly vanish!
        getchar(); 
#endif
        int* crash = NULL; *crash = 0; 
        return NULL;
    }
    
    return &g_asset_volumes[volume_index][local_offset];
}

#endif // !PLATFORM_H
