#ifndef GL_LOADER_H
#define GL_LOADER_H

/*
 * gl_loader.h - Manual OpenGL 3.3 core function pointer declarations
 *
 * No GLEW, no GLAD, no external dependencies. All proc addresses are loaded
 * at runtime via wglGetProcAddress (Windows) or glXGetProcAddress (Linux).
 *
 * Only the functions required for the fullscreen-quad blit pipeline are declared.
 * Include this header in both render_windows.c and render_linux.c.
 *
 * Call gl_loader_init() once after the GL context is current.
 */

#include <stdint.h>

/* -------------------------------------------------------------------------
 * Platform GL base includes
 * We include the system GL header for base types and GL 1.1 functions.
 * Everything above 1.1 comes from our own function pointers below.
 * ------------------------------------------------------------------------- */
#ifdef _WIN32
    #include <windows.h>
    #include <GL/gl.h>
#endif
#ifdef __linux__
    #include <GL/gl.h>
    #include <GL/glx.h>
#endif

/* -------------------------------------------------------------------------
 * GL type definitions
 * Only types not already provided by the system gl.h are defined here.
 * ------------------------------------------------------------------------- */
typedef char            GLchar;
typedef ptrdiff_t       GLsizeiptr;
typedef ptrdiff_t       GLintptr;

/* -------------------------------------------------------------------------
 * GL enumerants needed for the pipeline
 * ------------------------------------------------------------------------- */
#define GL_ARRAY_BUFFER                 0x8892
#define GL_STATIC_DRAW                  0x88E4
#define GL_FRAGMENT_SHADER              0x8B30
#define GL_VERTEX_SHADER                0x8B31
#define GL_COMPILE_STATUS               0x8B81
#define GL_LINK_STATUS                  0x8B82
#define GL_INFO_LOG_LENGTH              0x8B84
#define GL_TEXTURE0                     0x84C0
#define GL_TEXTURE1                     0x84C1
#define GL_CLAMP_TO_EDGE                0x812F
#define GL_BGRA                         0x80E1

/* -------------------------------------------------------------------------
 * Function pointer typedefs
 * ------------------------------------------------------------------------- */
typedef void     (*PFNGLGENVERTEXARRAYSPROC)    (GLsizei n, GLuint* arrays);
typedef void     (*PFNGLBINDVERTEXARRAYPROC)    (GLuint array);
typedef void     (*PFNGLGENBUFFERSPROC)         (GLsizei n, GLuint* buffers);
typedef void     (*PFNGLBINDBUFFERPROC)         (GLenum target, GLuint buffer);
typedef void     (*PFNGLBUFFERDATAPROC)         (GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void     (*PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
typedef void     (*PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef GLuint   (*PFNGLCREATESHADERPROC)       (GLenum type);
typedef void     (*PFNGLSHADERSOURCEPROC)       (GLuint shader, GLsizei count, const GLchar *const* string, const GLint* length);
typedef void     (*PFNGLCOMPILESHADERPROC)      (GLuint shader);
typedef void     (*PFNGLGETSHADERIVPROC)        (GLuint shader, GLenum pname, GLint* params);
typedef void     (*PFNGLGETSHADERINFOLOGPROC)   (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint   (*PFNGLCREATEPROGRAMPROC)      (void);
typedef void     (*PFNGLATTACHSHADERPROC)       (GLuint program, GLuint shader);
typedef void     (*PFNGLLINKPROGRAMPROC)        (GLuint program);
typedef void     (*PFNGLGETPROGRAMIVPROC)       (GLuint program, GLenum pname, GLint* params);
typedef void     (*PFNGLGETPROGRAMINFOLOGPROC)  (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void     (*PFNGLUSEPROGRAMPROC)         (GLuint program);
typedef void     (*PFNGLDELETESHADERPROC)       (GLuint shader);
typedef GLint    (*PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
typedef void     (*PFNGLUNIFORM1IPROC)          (GLint location, GLint v0);
typedef void     (*PFNGLACTIVETEXTUREPROC)      (GLenum texture);
typedef void     (*PFNGLDELETEPROGRAMPROC)      (GLuint program);

/* -------------------------------------------------------------------------
 * Extern declarations — defined in gl_loader.c
 * ------------------------------------------------------------------------- */
extern PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
extern PFNGLGENBUFFERSPROC              glGenBuffers;
extern PFNGLBINDBUFFERPROC              glBindBuffer;
extern PFNGLBUFFERDATAPROC              glBufferData;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
extern PFNGLCREATESHADERPROC            glCreateShader;
extern PFNGLSHADERSOURCEPROC            glShaderSource;
extern PFNGLCOMPILESHADERPROC           glCompileShader;
extern PFNGLGETSHADERIVPROC             glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC           glCreateProgram;
extern PFNGLATTACHSHADERPROC            glAttachShader;
extern PFNGLLINKPROGRAMPROC             glLinkProgram;
extern PFNGLGETPROGRAMIVPROC            glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC              glUseProgram;
extern PFNGLDELETESHADERPROC            glDeleteShader;
extern PFNGLDELETEPROGRAMPROC           glDeleteProgram;
extern PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
extern PFNGLUNIFORM1IPROC               glUniform1i;
#ifdef _WIN32
extern PFNGLACTIVETEXTUREPROC           glActiveTexture;
#endif

/* -------------------------------------------------------------------------
 * Initialiser — must be called once after the GL context is made current
 * ------------------------------------------------------------------------- */
void gl_loader_init(void);

#endif /* GL_LOADER_H */