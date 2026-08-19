#ifdef __linux__

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <time.h>
#include <unistd.h>

#include "platform/platform.h"
#include "core/version.h"
#include "generated/Icon.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

/*
 * X11Context is shared between platform_linux.c and render_linux.c.
 * Both files define this struct locally — they must remain identical.
 * If you ever add a field, update both.
 */
typedef struct
{
    Display* display;
    Window   window;
    GC       gc;
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

/*
 * GLX FBConfig attribute list — must match what render_linux.c requests.
 * We select the visual here so the window is created with the correct
 * visual for the GL context. XCreateSimpleWindow cannot do this.
 */
#define GLX_X_RENDERABLE    0x8012
#define GLX_DRAWABLE_TYPE   0x8010
#define GLX_WINDOW_BIT      0x00000001
#define GLX_RENDER_TYPE     0x8011
#define GLX_RGBA_BIT        0x00000001
#define GLX_X_VISUAL_TYPE   0x22
#define GLX_TRUE_COLOR      0x8002
#define GLX_DOUBLEBUFFER    5
#define GLX_RED_SIZE        8
#define GLX_GREEN_SIZE      9
#define GLX_BLUE_SIZE       10
#define GLX_ALPHA_SIZE      11
#define GLX_DEPTH_SIZE      12
#define GLX_STENCIL_SIZE    13

void platform_init(const PlatformWindowDesc* desc)
{
    display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "XOpenDisplay failed\n");
        return;
    }

    int screen = DefaultScreen(display);

    /*
     * Choose a GLX FBConfig with the same attributes render_linux.c uses.
     * We need the Visual from this FBConfig to create the window correctly —
     * a Visual mismatch between the window and the GL context will cause
     * glXMakeCurrent to fail with a BadMatch error.
     */
    const int fb_attribs[] = {
        GLX_X_RENDERABLE,  True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE,      8,
        GLX_GREEN_SIZE,    8,
        GLX_BLUE_SIZE,     8,
        GLX_ALPHA_SIZE,    8,
        GLX_DEPTH_SIZE,    24,
        GLX_STENCIL_SIZE,  8,
        GLX_DOUBLEBUFFER,  True,
        None
    };

    int fbc_count = 0;
    GLXFBConfig* fbcs = glXChooseFBConfig(display, screen, fb_attribs, &fbc_count);
    if (!fbcs || fbc_count == 0)
    {
        fprintf(stderr, "platform_linux: no suitable GLX FBConfig\n");
        XCloseDisplay(display);
        return;
    }

    /* Pick best depth, same heuristic as render_linux.c */
    GLXFBConfig best_fbc = fbcs[0];
    int best_depth = 0;
    for (int i = 0; i < fbc_count; i++)
    {
        XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbcs[i]);
        if (vi)
        {
            if (vi->depth > best_depth)
            {
                best_depth = vi->depth;
                best_fbc   = fbcs[i];
            }
            XFree(vi);
        }
    }
    XFree(fbcs);

    XVisualInfo* vi = glXGetVisualFromFBConfig(display, best_fbc);
    if (!vi)
    {
        fprintf(stderr, "platform_linux: glXGetVisualFromFBConfig failed\n");
        XCloseDisplay(display);
        return;
    }

    /*
     * Create a Colormap for the chosen visual — required by XCreateWindow
     * when the visual differs from the root window's default visual.
     */
    Colormap cmap = XCreateColormap(
        display, RootWindow(display, vi->screen),
        vi->visual, AllocNone);

    XSetWindowAttributes swa = { 0 };
    swa.colormap     = cmap;
    swa.border_pixel = 0;  /* Required when Visual depth differs from root — omitting causes BadMatch */
    swa.event_mask   = ExposureMask | KeyPress | KeyRelease | StructureNotifyMask;

    /*
     * XCreateWindow replaces XCreateSimpleWindow.
     * The depth and visual come from the FBConfig — this is the key change.
     * CWBorderPixel must be set explicitly when using a non-default Visual depth.
     */
    window = XCreateWindow(
        display, RootWindow(display, vi->screen),
        10, 10, desc->width, desc->height, 0,
        vi->depth, InputOutput, vi->visual,
        CWColormap | CWBorderPixel | CWEventMask, &swa);

    XFree(vi);

    char title[128];
    snprintf(title, sizeof(title), "%s v%s", desc->title, gb_version);
    XStoreName(display, window, title);

    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wm_delete_window, 1);

    gc = XCreateGC(display, window, 0, NULL);

    XMapWindow(display, window);

    int64_t net_icon[2 + ICON_WIDTH * ICON_HEIGHT];
    net_icon[0] = ICON_WIDTH;
    net_icon[1] = ICON_HEIGHT;
    for (int i = 0; i < ICON_WIDTH * ICON_HEIGHT; i++)
        net_icon[2 + i] = (int64_t)icon_argb[i];

    Atom net_wm_icon = XInternAtom(display, "_NET_WM_ICON", False);
    XChangeProperty(display, window, net_wm_icon, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)net_icon,
                    2 + ICON_WIDTH * ICON_HEIGHT);

    XFlush(display);

    x11.display = display;
    x11.window  = window;
    x11.gc      = gc;

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

    double elapsed = (now.tv_sec - last_time.tv_sec)
                   + (now.tv_nsec - last_time.tv_nsec) / 1e9;
    double frame_target = 1.0 / TARGET_FPS;

    if (elapsed < frame_target)
    {
        usleep((useconds_t)((frame_target - elapsed) * 1e6));
        clock_gettime(CLOCK_MONOTONIC, &now);
        elapsed = (now.tv_sec - last_time.tv_sec)
                + (now.tv_nsec - last_time.tv_nsec) / 1e9;
    }

    last_time = now;
    return (float)elapsed;
}

void platform_set_window_title(const char* title)
{
    if (display && window)
    {
        XStoreName(display, window, title);
        XFlush(display);
    }
}

void platform_init_assets(int total_volumes)
{
    for (int i = 0; i < total_volumes && i < MAX_VOLUMES; i++)
    {
        char filename[64];
        snprintf(filename, sizeof(filename), "data/data_%03d.dat", i);

        int fd = open(filename, O_RDONLY);
        if (fd < 0) continue;

        g_volume_fds[i] = fd;

        void* mapped_address = mmap(NULL, VOLUME_SIZE, PROT_READ, MAP_PRIVATE, fd, 0);
        if (mapped_address == MAP_FAILED)
        {
            close(fd);
            g_volume_fds[i] = -1;
            continue;
        }

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

#endif /* __linux__ */