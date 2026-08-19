#ifdef __YGGDRASIL__
#include "platform/platform.h"
#include <yggdrasil.h>

static bool     g_running    = false;
static uint64_t g_last_time  = 0;

void platform_init(const PlatformWindowDesc *desc) {
    __asm__ volatile("sti");  // re-enable interrupts
    (void)desc;
    /* Kernel already set up the framebuffer — nothing to open */
    g_running   = true;
    g_last_time = pit_millis();
}

void platform_shutdown(void) {
    g_running = false;
}

void platform_poll_events(void) {
    /* Input is IRQ-driven — nothing to poll */
    /* SELECT held for 3 seconds = quit, matching Escape on Linux */
    static uint64_t select_held_since = 0;
    if (ps2_get_buttons() & YGG_BTN_SELECT) {
        if (select_held_since == 0)
            select_held_since = pit_millis();
        else if (pit_millis() - select_held_since > 3000)
            g_running = false;
    } else {
        select_held_since = 0;
    }
}

bool platform_running(void) {
    return g_running;
}

float platform_frame_timing(void) {
    /* Initialize on first call */
    if (g_last_time == 0)
        g_last_time = pit_millis();

    uint64_t target = (uint64_t)(1000.0f / TARGET_FPS);

    // while (pit_millis() - g_last_time < target)
    //     __asm__ volatile("hlt");

    uint64_t now = pit_millis();
    uint64_t elapsed = now - g_last_time;
    g_last_time = now;

    return (float)elapsed / 1000.0f;
}

void *platform_get_native_window(void) {
    /* Return the kernel framebuffer — render_init casts this */
    YggFramebuffer *fb = ygg_get_framebuffer();
    return (void *)fb;
}
#endif /* __YGGDRASIL__ */