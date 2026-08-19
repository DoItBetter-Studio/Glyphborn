/*
 * gl_loader.c - OpenGL 3.3 core function pointer definitions and loader
 *
 * Platform-specific proc address retrieval:
 *   Windows : wglGetProcAddress  (opengl32.lib only exposes up to GL 1.1 directly)
 *   Linux   : glXGetProcAddress  (covers all GL versions including base)
 *
 * Add new function pointers here if the pipeline grows.
 */

#include "render/gl_loader.h"
#include <stdio.h>
#include <assert.h>

/* -------------------------------------------------------------------------
 * Function pointer storage
 * ------------------------------------------------------------------------- */
PFNGLGENVERTEXARRAYSPROC            glGenVertexArrays           = NULL;
PFNGLBINDVERTEXARRAYPROC            glBindVertexArray           = NULL;
PFNGLGENBUFFERSPROC                 glGenBuffers                = NULL;
PFNGLBINDBUFFERPROC                 glBindBuffer                = NULL;
PFNGLBUFFERDATAPROC                 glBufferData                = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC    glEnableVertexAttribArray   = NULL;
PFNGLVERTEXATTRIBPOINTERPROC        glVertexAttribPointer       = NULL;
PFNGLCREATESHADERPROC               glCreateShader              = NULL;
PFNGLSHADERSOURCEPROC               glShaderSource              = NULL;
PFNGLCOMPILESHADERPROC              glCompileShader             = NULL;
PFNGLGETSHADERIVPROC                glGetShaderiv               = NULL;
PFNGLGETSHADERINFOLOGPROC           glGetShaderInfoLog          = NULL;
PFNGLCREATEPROGRAMPROC              glCreateProgram             = NULL;
PFNGLATTACHSHADERPROC               glAttachShader              = NULL;
PFNGLLINKPROGRAMPROC                glLinkProgram               = NULL;
PFNGLGETPROGRAMIVPROC               glGetProgramiv              = NULL;
PFNGLGETPROGRAMINFOLOGPROC          glGetProgramInfoLog         = NULL;
PFNGLUSEPROGRAMPROC                 glUseProgram                = NULL;
PFNGLDELETESHADERPROC               glDeleteShader              = NULL;
PFNGLDELETEPROGRAMPROC              glDeleteProgram             = NULL;
PFNGLGETUNIFORMLOCATIONPROC         glGetUniformLocation        = NULL;
PFNGLUNIFORM1IPROC                  glUniform1i                 = NULL;
PFNGLUNIFORMMATRIX4FVPROC           glUniformMatrix4fv          = NULL;
PFNGLUNIFORM1FPROC                  glUniform1f                 = NULL;
PFNGLUNIFORM3FPROC                  glUniform3f                 = NULL;
PFNGLUNIFORM4FPROC                  glUniform4f                 = NULL;
PFNGLDELETEBUFFERSPROC              glDeleteBuffers             = NULL;
PFNGLDELETEVERTEXARRAYSPROC         glDeleteVertexArrays        = NULL;
#ifdef _WIN32
PFNGLACTIVETEXTUREPROC              glActiveTexture             = NULL;
#endif
/* -------------------------------------------------------------------------
 * Platform proc address helper
 * ------------------------------------------------------------------------- */
#ifdef _WIN32
static void* get_proc(const char* name)
{
    void* p = (void*)wglGetProcAddress(name);
    
    // If wglGetProcAddress fails, it might be an older base function built into the DLL
    if (!p || (p == (void*)1) || (p == (void*)2) || (p == (void*)3) || (p == (void*)-1))
    {
        HMODULE module = GetModuleHandleA("opengl32.dll");
        p = (void*)GetProcAddress(module, name);
    }
    
    if (!p)
    {
        fprintf(stderr, "gl_loader: failed to load %s\n", name);
    }
    return p;
}
#endif

#ifdef __linux__
static void* get_proc(const char* name)
{
    void* p = (void*)glXGetProcAddress((const GLubyte*)name);
    if (!p)
    {
        fprintf(stderr, "gl_loader: failed to load %s\n", name);
    }
    return p;
}
#endif

/* -------------------------------------------------------------------------
 * gl_loader_init - Load all function pointers
 * Must be called once after the GL 3.3 context is current.
 * ------------------------------------------------------------------------- */
void gl_loader_init(void)
{
    glGenVertexArrays           = (PFNGLGENVERTEXARRAYSPROC)            get_proc("glGenVertexArrays");
    glBindVertexArray           = (PFNGLBINDVERTEXARRAYPROC)            get_proc("glBindVertexArray");
    glGenBuffers                = (PFNGLGENBUFFERSPROC)                 get_proc("glGenBuffers");
    glBindBuffer                = (PFNGLBINDBUFFERPROC)                 get_proc("glBindBuffer");
    glBufferData                = (PFNGLBUFFERDATAPROC)                 get_proc("glBufferData");
    glEnableVertexAttribArray   = (PFNGLENABLEVERTEXATTRIBARRAYPROC)    get_proc("glEnableVertexAttribArray");
    glVertexAttribPointer       = (PFNGLVERTEXATTRIBPOINTERPROC)        get_proc("glVertexAttribPointer");
    glCreateShader              = (PFNGLCREATESHADERPROC)               get_proc("glCreateShader");
    glShaderSource              = (PFNGLSHADERSOURCEPROC)               get_proc("glShaderSource");
    glCompileShader             = (PFNGLCOMPILESHADERPROC)              get_proc("glCompileShader");
    glGetShaderiv               = (PFNGLGETSHADERIVPROC)                get_proc("glGetShaderiv");
    glGetShaderInfoLog          = (PFNGLGETSHADERINFOLOGPROC)           get_proc("glGetShaderInfoLog");
    glCreateProgram             = (PFNGLCREATEPROGRAMPROC)              get_proc("glCreateProgram");
    glAttachShader              = (PFNGLATTACHSHADERPROC)               get_proc("glAttachShader");
    glLinkProgram               = (PFNGLLINKPROGRAMPROC)                get_proc("glLinkProgram");
    glGetProgramiv              = (PFNGLGETPROGRAMIVPROC)               get_proc("glGetProgramiv");
    glGetProgramInfoLog         = (PFNGLGETPROGRAMINFOLOGPROC)          get_proc("glGetProgramInfoLog");
    glUseProgram                = (PFNGLUSEPROGRAMPROC)                 get_proc("glUseProgram");
    glDeleteShader              = (PFNGLDELETESHADERPROC)               get_proc("glDeleteShader");
    glDeleteProgram             = (PFNGLDELETEPROGRAMPROC)              get_proc("glDeleteProgram");
    glGetUniformLocation        = (PFNGLGETUNIFORMLOCATIONPROC)         get_proc("glGetUniformLocation");
    glUniform1i                 = (PFNGLUNIFORM1IPROC)                  get_proc("glUniform1i");
    glUniformMatrix4fv          = (PFNGLUNIFORMMATRIX4FVPROC)           get_proc("glUniformMatrix4fv");
    glUniform1f                 = (PFNGLUNIFORM1FPROC)                  get_proc("glUniform1f");
    glUniform3f                 = (PFNGLUNIFORM3FPROC)                  get_proc("glUniform3f");
    glUniform4f                 = (PFNGLUNIFORM4FPROC)                  get_proc("glUniform4f");
    glDeleteBuffers             = (PFNGLDELETEBUFFERSPROC)              get_proc("glDeleteBuffers");
    glDeleteVertexArrays        = (PFNGLDELETEVERTEXARRAYSPROC)         get_proc("glDeleteVertexArrays");
#ifdef _WIN32
    glActiveTexture             = (PFNGLACTIVETEXTUREPROC)              get_proc("glActiveTexture");
#endif
}