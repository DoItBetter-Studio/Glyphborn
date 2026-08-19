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
#define GL_ELEMENT_ARRAY_BUFFER         0x8893
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
#define GL_RGBA8                        0x8058
#define GL_UNSIGNED_BYTE                0x1401
#define GL_UNSIGNED_SHORT               0x1403
#define GL_UNSIGNED_INT                 0x1405
#define GL_FLOAT                        0x1406
#define GL_FALSE                        0
#define GL_TEXTURE_2D                   0x0DE1
#define GL_TEXTURE_MIN_FILTER           0x2801
#define GL_TEXTURE_MAG_FILTER           0x2800
#define GL_TEXTURE_WRAP_S               0x2802
#define GL_TEXTURE_WRAP_T               0x2803
#define GL_NEAREST                      0x2600
#define GL_TRIANGLES                    0x0004
#define GL_RGBA                         0x1908

/* -------------------------------------------------------------------------
 * Function pointer typedefs
 * ------------------------------------------------------------------------- */
typedef void        (APIENTRY *PFNGLGENVERTEXARRAYSPROC)                (GLsizei n, GLuint* arrays);
typedef void        (APIENTRY *PFNGLBINDVERTEXARRAYPROC)                (GLuint array);
typedef void        (APIENTRY *PFNGLGENBUFFERSPROC)                     (GLsizei n, GLuint* buffers);
typedef void        (APIENTRY *PFNGLBINDBUFFERPROC)                     (GLenum target, GLuint buffer);
typedef void        (APIENTRY *PFNGLBUFFERDATAPROC)                     (GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void        (APIENTRY *PFNGLENABLEVERTEXATTRIBARRAYPROC)        (GLuint index);
typedef void        (APIENTRY *PFNGLVERTEXATTRIBPOINTERPROC)            (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef GLuint      (APIENTRY *PFNGLCREATESHADERPROC)                   (GLenum type);
typedef void        (APIENTRY *PFNGLSHADERSOURCEPROC)                   (GLuint shader, GLsizei count, const GLchar *const* string, const GLint* length);
typedef void        (APIENTRY *PFNGLCOMPILESHADERPROC)                  (GLuint shader);
typedef void        (APIENTRY *PFNGLGETSHADERIVPROC)                    (GLuint shader, GLenum pname, GLint* params);
typedef void        (APIENTRY *PFNGLGETSHADERINFOLOGPROC)               (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLuint      (APIENTRY *PFNGLCREATEPROGRAMPROC)                  (void);
typedef void        (APIENTRY *PFNGLATTACHSHADERPROC)                   (GLuint program, GLuint shader);
typedef void        (APIENTRY *PFNGLLINKPROGRAMPROC)                    (GLuint program);
typedef void        (APIENTRY *PFNGLGETPROGRAMIVPROC)                   (GLuint program, GLenum pname, GLint* params);
typedef void        (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC)              (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void        (APIENTRY *PFNGLUSEPROGRAMPROC)                     (GLuint program);
typedef void        (APIENTRY *PFNGLDELETESHADERPROC)                   (GLuint shader);
typedef GLint       (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC)             (GLuint program, const GLchar* name);
typedef void        (APIENTRY *PFNGLUNIFORM1IPROC)                      (GLint location, GLint v0);
typedef void        (APIENTRY *PFNGLUNIFORM1FPROC)                      (GLint location, float v0);
typedef void        (APIENTRY *PFNGLUNIFORM3FPROC)                      (GLint location, float v0, float v1, float v2);
typedef void        (APIENTRY *PFNGLUNIFORM4FPROC)                      (GLint location, float v0, float v1, float v2, float v3);
typedef void        (APIENTRY *PFNGLACTIVETEXTUREPROC)                  (GLenum texture);
typedef void        (APIENTRY *PFNGLDELETEPROGRAMPROC)                  (GLuint program);
typedef void        (APIENTRY *PFNGLUNIFORMMATRIX4FVPROC)               (GLint location, GLsizei count, GLboolean transpose, const float* value);
typedef void        (APIENTRY *PFNGLDELETEBUFFERSPROC)                  (GLsizei n, const GLuint* buffers);
typedef void        (APIENTRY *PFNGLDELETEVERTEXARRAYSPROC)             (GLsizei n, const GLuint* arrays);
typedef void        (APIENTRY *PFNGLDRAWELEMENTSPROC)                   (GLenum mode, GLsizei count, GLenum type, const void* indices);
typedef void        (APIENTRY *PFNGLBINDTEXTUREPROC)                    (GLenum target, GLuint texture);
typedef void        (APIENTRY *PFNGLGENTEXTURESPROC)                    (GLsizei n, GLuint* textures);
typedef void        (APIENTRY *PFNGLTEXIMAGE2DPROC)                     (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels);
typedef void        (APIENTRY *PFNGLTEXPARAMETERIPROC)                  (GLenum target, GLenum pname, GLint param);

/* -------------------------------------------------------------------------
 * Extern declarations — defined in gl_loader.c
 * ------------------------------------------------------------------------- */
extern PFNGLGENVERTEXARRAYSPROC             glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC             glBindVertexArray;
extern PFNGLGENBUFFERSPROC                  glGenBuffers;
extern PFNGLBINDBUFFERPROC                  glBindBuffer;
extern PFNGLBUFFERDATAPROC                  glBufferData;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC     glEnableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC         glVertexAttribPointer;
extern PFNGLCREATESHADERPROC                glCreateShader;
extern PFNGLSHADERSOURCEPROC                glShaderSource;
extern PFNGLCOMPILESHADERPROC               glCompileShader;
extern PFNGLGETSHADERIVPROC                 glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC            glGetShaderInfoLog;
extern PFNGLCREATEPROGRAMPROC               glCreateProgram;
extern PFNGLATTACHSHADERPROC                glAttachShader;
extern PFNGLLINKPROGRAMPROC                 glLinkProgram;
extern PFNGLGETPROGRAMIVPROC                glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC           glGetProgramInfoLog;
extern PFNGLUSEPROGRAMPROC                  glUseProgram;
extern PFNGLDELETESHADERPROC                glDeleteShader;
extern PFNGLDELETEPROGRAMPROC               glDeleteProgram;
extern PFNGLGETUNIFORMLOCATIONPROC          glGetUniformLocation;
extern PFNGLUNIFORM1IPROC                   glUniform1i;
extern PFNGLUNIFORMMATRIX4FVPROC            glUniformMatrix4fv;
extern PFNGLUNIFORM1FPROC                   glUniform1f;
extern PFNGLUNIFORM3FPROC                   glUniform3f;
extern PFNGLUNIFORM4FPROC                   glUniform4f;
extern PFNGLDELETEBUFFERSPROC               glDeleteBuffers;
extern PFNGLDELETEVERTEXARRAYSPROC          glDeleteVertexArrays;
#ifdef _WIN32
extern PFNGLACTIVETEXTUREPROC               glActiveTexture;
#endif

/* -------------------------------------------------------------------------
 * Initialiser — must be called once after the GL context is made current
 * ------------------------------------------------------------------------- */
void gl_loader_init(void);

#endif /* GL_LOADER_H */