#ifdef _WIN32

/*
 * render_windows.c - Win32 OpenGL 3.3 core rendering backend
 *
 * Replaces the GDI StretchDIBits software blit with a proper OpenGL 3.3 core
 * context and a fullscreen-quad pipeline. The CPU-side framebuffers are
 * unchanged — framebuffer_game and framebuffer_ui are still written by the
 * software rasterizer and UI system each frame. This file uploads them as
 * GL textures and composites them in the fragment shader, replacing the CPU
 * render_blend_ui_over_game() call.
 *
 * Context creation:
 *   Win32 requires a two-pass context: a dummy legacy context is created first
 *   solely to get wglCreateContextAttribsARB, which is then used to create the
 *   real GL 3.3 core profile context. The dummy context and its dummy HWND are
 *   destroyed immediately after.
 *
 * Pixel format:
 *   framebuffer_game and framebuffer_ui are 0xAARRGGBB in memory.
 *   GL_BGRA + GL_UNSIGNED_BYTE maps directly to this layout with no swizzle.
 *
 * Blend:
 *   render_blend_ui_over_game() is kept as a no-op stub. The fragment shader
 *   performs the identical alpha composite. framebuffer (the old merged target)
 *   is retained in memory but never uploaded to GL.
 */

#include "render.h"
#include "gl_loader.h"
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * wgl extension types and enumerants
 * Only what we need — no wglext.h dependency.
 * ------------------------------------------------------------------------- */
typedef HGLRC (WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC)(HDC hDC, HGLRC hShareContext, const int* attribList);
typedef BOOL  (WINAPI* PFNWGLCHOOSEPIXELFORMATARBPROC)(HDC hdc, const int* piAttribIList, const FLOAT* pfAttribFList, UINT nMaxFormats, int* piFormats, UINT* nNumFormats);

#define WGL_CONTEXT_MAJOR_VERSION_ARB   0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB   0x2092
#define WGL_CONTEXT_PROFILE_MASK_ARB    0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x0002

#define WGL_DRAW_TO_WINDOW_ARB          0x2001
#define WGL_SUPPORT_OPENGL_ARB          0x2010
#define WGL_DOUBLE_BUFFER_ARB           0x2011
#define WGL_PIXEL_TYPE_ARB              0x2013
#define WGL_TYPE_RGBA_ARB               0x202B
#define WGL_COLOR_BITS_ARB              0x2014
#define WGL_DEPTH_BITS_ARB              0x2022
#define WGL_STENCIL_BITS_ARB            0x2023
#define WGL_ACCELERATION_ARB            0x2003
#define WGL_FULL_ACCELERATION_ARB       0x2027

/* -------------------------------------------------------------------------
 * Framebuffer storage
 * framebuffer is the legacy merged target — kept for API compatibility but
 * not uploaded to GL. game and ui are the two live texture sources.
 * ------------------------------------------------------------------------- */
uint32_t framebuffer[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_game[FB_WIDTH * FB_HEIGHT];
uint32_t framebuffer_ui[FB_WIDTH * FB_HEIGHT];
float    depthbuffer[FB_WIDTH * FB_HEIGHT];

/* -------------------------------------------------------------------------
 * GL state
 * ------------------------------------------------------------------------- */
static HDC   s_hdc   = NULL;
static HGLRC s_hglrc = NULL;

static GLuint s_vao         = 0;
static GLuint s_vbo         = 0;
static GLuint s_program     = 0;
static GLuint s_tex_ui      = 0;
static GLint  s_loc_ui      = -1;

/* -------------------------------------------------------------------------
 * Fullscreen quad — NDC coords + UVs, no matrix needed
 *
 * Two triangles covering [-1,1] in X and Y.
 * UV origin is top-left to match the framebuffer layout (row 0 = top).
 * ------------------------------------------------------------------------- */
static const float s_quad[] = {
/*   X      Y     U     V  */
    -1.0f,  1.0f, 0.0f, 0.0f,   /* top-left     */
    -1.0f, -1.0f, 0.0f, 1.0f,   /* bottom-left  */
     1.0f, -1.0f, 1.0f, 1.0f,   /* bottom-right */

    -1.0f,  1.0f, 0.0f, 0.0f,   /* top-left     */
     1.0f, -1.0f, 1.0f, 1.0f,   /* bottom-right */
     1.0f,  1.0f, 1.0f, 0.0f,   /* top-right    */
};

/* -------------------------------------------------------------------------
 * Shaders
 *
 * Vertex: pass-through, no transforms.
 * Fragment: sample game and UI textures, alpha-composite UI over game.
 *
 * The composite matches render_blend_ui_over_game() exactly:
 *   if ui.a == 0   -> output game pixel
 *   if ui.a == 1   -> output ui pixel
 *   else           -> lerp(game, ui, ui.a)
 *
 * GL_BGRA upload means the sampler sees (r=R, g=G, b=B, a=A) correctly —
 * no channel fixup needed in the shader.
 * ------------------------------------------------------------------------- */
static const char* s_vert_src =
    "#version 330 core\n"
    "layout(location = 0) in vec2 a_pos;\n"
    "layout(location = 1) in vec2 a_uv;\n"
    "out vec2 v_uv;\n"
    "void main() {\n"
    "    gl_Position = vec4(a_pos, 0.0, 1.0);\n"
    "    v_uv = a_uv;\n"
    "}\n";

static const char* s_frag_src =
    "#version 330 core\n"
    "in vec2 v_uv;\n"
    "out vec4 frag_color;\n"
    "uniform sampler2D u_ui;\n"
    "void main() {\n"
    "    frag_color = texture(u_ui, v_uv);\n"
    "}\n";

/* -------------------------------------------------------------------------
 * Internal helpers
 * ------------------------------------------------------------------------- */
static GLuint compile_shader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        char* log = (char*)_alloca(len + 1);
        glGetShaderInfoLog(shader, len, NULL, log);
        fprintf(stderr, "render_windows: shader compile error:\n%s\n", log);
    }
    return shader;
}

static GLuint create_texture(void)
{
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    /* Allocate storage — data uploaded each frame via glTexSubImage2D */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 FB_WIDTH, FB_HEIGHT, 0,
                 GL_BGRA, GL_UNSIGNED_BYTE, NULL);
    return tex;
}

/* -------------------------------------------------------------------------
 * render_init
 *
 * Two-pass WGL context creation:
 *   Pass 1 — dummy HWND + legacy context to get wglCreateContextAttribsARB
 *   Pass 2 — real context on the actual HWND at GL 3.3 core
 * ------------------------------------------------------------------------- */
void render_init(void* platform_context)
{
    HWND hwnd = (HWND)platform_context;

    /* --- Pass 1: dummy context to load WGL extensions ------------------- */
    WNDCLASSA dummy_wc = { 0 };
    dummy_wc.lpfnWndProc   = DefWindowProcA;
    dummy_wc.hInstance     = GetModuleHandle(NULL);
    dummy_wc.lpszClassName = "glyphborn_dummy_wc";
    RegisterClassA(&dummy_wc);

    HWND dummy_hwnd = CreateWindowA(
        dummy_wc.lpszClassName, "dummy",
        WS_OVERLAPPED,
        0, 0, 1, 1,
        NULL, NULL, dummy_wc.hInstance, NULL);

    HDC dummy_dc = GetDC(dummy_hwnd);

    PIXELFORMATDESCRIPTOR pfd = { 0 };
    pfd.nSize      = sizeof(pfd);
    pfd.nVersion   = 1;
    pfd.dwFlags    = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;

    int dummy_fmt = ChoosePixelFormat(dummy_dc, &pfd);
    SetPixelFormat(dummy_dc, dummy_fmt, &pfd);

    HGLRC dummy_ctx = wglCreateContext(dummy_dc);
    wglMakeCurrent(dummy_dc, dummy_ctx);

    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
        (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
        (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(dummy_ctx);
    ReleaseDC(dummy_hwnd, dummy_dc);
    DestroyWindow(dummy_hwnd);

    if (!wglCreateContextAttribsARB || !wglChoosePixelFormatARB)
    {
        fprintf(stderr, "render_windows: WGL extensions unavailable — GL 3.3 not supported\n");
        return;
    }

    /* --- Pass 2: real context on the game window ------------------------ */
    s_hdc = GetDC(hwnd);

    const int pf_attribs[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_ACCELERATION_ARB,   WGL_FULL_ACCELERATION_ARB,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     32,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        0
    };

    int pixel_format = 0;
    UINT num_formats  = 0;
    wglChoosePixelFormatARB(s_hdc, pf_attribs, NULL, 1, &pixel_format, &num_formats);

    PIXELFORMATDESCRIPTOR chosen_pfd = { 0 };
    DescribePixelFormat(s_hdc, pixel_format, sizeof(chosen_pfd), &chosen_pfd);
    SetPixelFormat(s_hdc, pixel_format, &chosen_pfd);

    const int ctx_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB,  3,
        WGL_CONTEXT_MINOR_VERSION_ARB,  3,
        WGL_CONTEXT_PROFILE_MASK_ARB,   WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB,          WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0
    };

    s_hglrc = wglCreateContextAttribsARB(s_hdc, NULL, ctx_attribs);
    if (!s_hglrc)
    {
        fprintf(stderr, "render_windows: failed to create GL 3.3 core context\n");
        return;
    }

    wglMakeCurrent(s_hdc, s_hglrc);

    /* --- Load GL 3.3 function pointers ---------------------------------- */
    gl_loader_init();

    /* --- Build the fullscreen quad VAO/VBO ------------------------------ */
    glGenVertexArrays(1, &s_vao);
    glBindVertexArray(s_vao);

    glGenBuffers(1, &s_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_quad), s_quad, GL_STATIC_DRAW);

    /* layout(location=0) vec2 a_pos */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);

    /* layout(location=1) vec2 a_uv */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    /* --- Compile and link shaders --------------------------------------- */
    GLuint vert = compile_shader(GL_VERTEX_SHADER,   s_vert_src);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, s_frag_src);

    s_program = glCreateProgram();
    glAttachShader(s_program, vert);
    glAttachShader(s_program, frag);
    glLinkProgram(s_program);

    GLint link_ok = 0;
    glGetProgramiv(s_program, GL_LINK_STATUS, &link_ok);
    if (!link_ok)
    {
        GLint len = 0;
        glGetProgramiv(s_program, GL_INFO_LOG_LENGTH, &len);
        char* log = (char*)_alloca(len + 1);
        glGetProgramInfoLog(s_program, len, NULL, log);
        fprintf(stderr, "render_windows: program link error:\n%s\n", log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    glUseProgram(s_program);
    s_loc_ui = glGetUniformLocation(s_program, "u_ui");
    glUniform1i(s_loc_ui, 0);   /* texture unit 0 */

    /* --- Allocate UI framebuffer texture -------------------------------- */
    s_tex_ui = create_texture();
}

/* -------------------------------------------------------------------------
 * render_get_framebuffer
 * ------------------------------------------------------------------------- */
uint32_t* render_get_framebuffer(void)
{
    return framebuffer;
}

/* -------------------------------------------------------------------------
 * render_clear
 * ------------------------------------------------------------------------- */
void render_clear(uint32_t* buffer, uint32_t color)
{
    for (int i = 0; i < FB_WIDTH * FB_HEIGHT; ++i)
        buffer[i] = color;
}

/* -------------------------------------------------------------------------
 * render_blend_ui_over_game
 *
 * Intentional no-op. The GPU fragment shader performs this composite.
 * Kept so main.c and the call site require no changes.
 * ------------------------------------------------------------------------- */
void render_blend_ui_over_game(void)
{
    /* no-op: composite happens in the fragment shader */
}

/* -------------------------------------------------------------------------
 * render_present
 *
 * 1. Upload both CPU framebuffers to their GL textures
 * 2. Set viewport to match the window client area (handles resize)
 * 3. Draw the fullscreen quad — shader composites game + UI
 * 4. Swap buffers
 * ------------------------------------------------------------------------- */
void render_present(void)
{
    if (!s_hdc || !s_hglrc) return;

    /* Sync viewport to window size every frame — handles resize for free */
    RECT rect;
    GetClientRect(WindowFromDC(s_hdc), &rect);
    glViewport(0, 0, rect.right - rect.left, rect.bottom - rect.top);

    /*
     * The world geometry is already in the backbuffer from world_render.
     * We only upload and composite the UI layer on top of it.
     * GL_BLEND with standard src-alpha lets UI pixels with alpha=0 pass
     * through to the world geometry underneath.
     */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_tex_ui);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                    FB_WIDTH, FB_HEIGHT,
                    GL_BGRA, GL_UNSIGNED_BYTE,
                    framebuffer_ui);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(s_program);
    glBindVertexArray(s_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_BLEND);

    SwapBuffers(s_hdc);
}

/* -------------------------------------------------------------------------
 * render_shutdown
 * ------------------------------------------------------------------------- */
void render_shutdown(void)
{
    if (s_hglrc)
    {
        glDeleteTextures(1, &s_tex_ui);
        glDeleteProgram(s_program);
        glBindVertexArray(0);
        /* VAO and VBO cleanup */
        GLuint bufs[] = { s_vbo };
        /* glDeleteBuffers is GL 1.5 — available via proc address */
        /* For correctness we call through the loader */
        typedef void (*PFNGLDELETEBUFFERSPROC)(GLsizei, const GLuint*);
        PFNGLDELETEBUFFERSPROC glDeleteBuffers =
            (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
        if (glDeleteBuffers) glDeleteBuffers(1, bufs);

        typedef void (*PFNGLDELETEVERTEXARRAYSPROC)(GLsizei, const GLuint*);
        PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays =
            (PFNGLDELETEVERTEXARRAYSPROC)wglGetProcAddress("glDeleteVertexArrays");
        if (glDeleteVertexArrays) glDeleteVertexArrays(1, &s_vao);

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(s_hglrc);
        s_hglrc = NULL;
    }

    if (s_hdc)
    {
        HWND hwnd = WindowFromDC(s_hdc);
        ReleaseDC(hwnd, s_hdc);
        s_hdc = NULL;
    }
}

#endif /* _WIN32 */