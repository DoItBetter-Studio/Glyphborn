/*
 * world_render_gl.c - OpenGL 3.3 chunk rendering pipeline
 *
 * One VAO draw per chunk (9 max visible at a time). The fragment shader
 * performs per-pixel Lambert diffuse + ambient using the live sun uniforms,
 * so day/night changes take effect without any mesh rebuild.
 *
 * Normal transform: for this engine, chunk model matrices are pure
 * translations — no rotation, no non-uniform scale — so the normal matrix
 * is identity and we can use the object-space normal directly in the
 * fragment shader after multiplying by the model matrix's upper 3x3.
 * If you ever add rotated/scaled chunks, pass a proper normal matrix.
 */

#include "world/world_render_gl.h"
#include "gl_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Additional GL enumerants
 * ------------------------------------------------------------------------- */
#define GL_ELEMENT_ARRAY_BUFFER  0x8893
#define GL_TRIANGLES             0x0004
#define GL_UNSIGNED_SHORT        0x1403
#define GL_TEXTURE_2D            0x0DE1
#define GL_FALSE                 0

typedef void (*PFNGLUNIFORMMATRIX4FVPROC)(GLint, GLsizei, GLboolean, const float*);
typedef void (*PFNGLUNIFORM3FPROC)       (GLint, float, float, float);
typedef void (*PFNGLUNIFORM1FPROC)       (GLint, float);

static PFNGLUNIFORMMATRIX4FVPROC s_glUniformMatrix4fv = NULL;
static PFNGLUNIFORM3FPROC        s_glUniform3f        = NULL;
static PFNGLUNIFORM1FPROC        s_glUniform1f        = NULL;

/* -------------------------------------------------------------------------
 * Shader source
 *
 * Vertex: transforms position by MVP, passes world-space normal and UV.
 * Fragment: nearest-neighbour atlas sample + Lambert diffuse + ambient.
 *
 * The sun direction uniform is in world space — no need to transform it
 * since normals are also kept in world space (translation-only model).
 * ------------------------------------------------------------------------- */
static const char* s_vert =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec2 a_uv;\n"
    "layout(location = 2) in vec3 a_normal;\n"
    "\n"
    "uniform mat4 u_mvp;\n"
    "uniform mat4 u_model;\n"
    "\n"
    "out vec2  v_uv;\n"
    "flat out vec3  v_normal;\n"  /* flat: no interpolation — preserves per-face lighting */
    "\n"
    "void main() {\n"
    "    gl_Position = u_mvp * vec4(a_pos, 1.0);\n"
    "    v_uv        = a_uv;\n"
    "    v_normal    = mat3(u_model) * a_normal;\n"
    "}\n";

static const char* s_frag =
    "#version 330 core\n"
    "in vec2  v_uv;\n"
    "flat in vec3  v_normal;\n"  /* flat: matches vertex shader declaration */
    "out vec4 frag_color;\n"
    "\n"
    "uniform sampler2D u_atlas;\n"
    "uniform vec3  u_sun_dir;\n"
    "uniform float u_ambient;\n"
    "uniform float u_intensity;\n"
    "\n"
    "void main() {\n"
    "    vec4 tex = texture(u_atlas, v_uv);\n"
    "\n"
    /* No normalize needed — flat means the value is constant and already
     * normalised from gpu_mesh_upload. dot with -sun_dir because sun_dir
     * points away from the sun (toward ground), same as the CPU rasterizer. */
    "    float ndotl  = max(dot(v_normal, u_sun_dir), 0.0);\n"
    "    float light  = clamp(u_ambient + ndotl * u_intensity, 0.0, 1.0);\n"
    "\n"
    "    frag_color = vec4(tex.rgb * light, tex.a);\n"
    "}\n";

/* -------------------------------------------------------------------------
 * Pipeline state
 * ------------------------------------------------------------------------- */
static GLuint s_program  = 0;

static GLint  s_loc_mvp       = -1;
static GLint  s_loc_model     = -1;
static GLint  s_loc_atlas     = -1;
static GLint  s_loc_sun_dir   = -1;
static GLint  s_loc_ambient   = -1;
static GLint  s_loc_intensity = -1;

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
        char* log = malloc(len + 1);
        if (log)
        {
            glGetShaderInfoLog(shader, len, NULL, log);
            fprintf(stderr, "world_render_gl: shader error:\n%s\n", log);
            free(log);
        }
    }
    return shader;
}

static void load_extra_uniforms(void)
{
#ifdef _WIN32
    s_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");
    s_glUniform3f        = (PFNGLUNIFORM3FPROC)        wglGetProcAddress("glUniform3f");
    s_glUniform1f        = (PFNGLUNIFORM1FPROC)        wglGetProcAddress("glUniform1f");
#else
    s_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) glXGetProcAddress((const GLubyte*)"glUniformMatrix4fv");
    s_glUniform3f        = (PFNGLUNIFORM3FPROC)        glXGetProcAddress((const GLubyte*)"glUniform3f");
    s_glUniform1f        = (PFNGLUNIFORM1FPROC)        glXGetProcAddress((const GLubyte*)"glUniform1f");
#endif
}

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */
void world_gl_init(void)
{
    load_extra_uniforms();

    GLuint vert = compile_shader(GL_VERTEX_SHADER,   s_vert);
    GLuint frag = compile_shader(GL_FRAGMENT_SHADER, s_frag);

    s_program = glCreateProgram();
    glAttachShader(s_program, vert);
    glAttachShader(s_program, frag);
    glLinkProgram(s_program);

    GLint ok = 0;
    glGetProgramiv(s_program, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        GLint len = 0;
        glGetProgramiv(s_program, GL_INFO_LOG_LENGTH, &len);
        char* log = malloc(len + 1);
        if (log)
        {
            glGetProgramInfoLog(s_program, len, NULL, log);
            fprintf(stderr, "world_render_gl: link error:\n%s\n", log);
            free(log);
        }
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    glUseProgram(s_program);
    s_loc_mvp       = glGetUniformLocation(s_program, "u_mvp");
    s_loc_model     = glGetUniformLocation(s_program, "u_model");
    s_loc_atlas     = glGetUniformLocation(s_program, "u_atlas");
    s_loc_sun_dir   = glGetUniformLocation(s_program, "u_sun_dir");
    s_loc_ambient   = glGetUniformLocation(s_program, "u_ambient");
    s_loc_intensity = glGetUniformLocation(s_program, "u_intensity");

    glUniform1i(s_loc_atlas, 0);  /* texture unit 0 */

    /* Enable depth testing for the world pass */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);  /* tile meshes have no guaranteed winding order */
}

void world_gl_shutdown(void)
{
    if (s_program)
    {
        glDeleteProgram(s_program);
        s_program = 0;
    }
}

void world_gl_begin_pass(void)
{
    /* Assert all world pipeline state once per frame before any draw calls.
     * render_present disturbs depth test and blend state for the UI pass,
     * so we cannot rely on init-time state surviving to the next frame. */
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);
}

void world_gl_draw(const GPUMesh*          mesh,
                   Mat4                    model,
                   Mat4                    view,
                   Mat4                    projection,
                   const DirectionalLight* sun)
{
    if (!mesh || !mesh->vao || !mesh->index_count) return;
    if (!s_program || !s_glUniformMatrix4fv) return;

    /* MVP = projection * view * model */
    Mat4 mv  = mat4_multiply(view, model);
    Mat4 mvp = mat4_multiply(projection, mv);

    glUseProgram(s_program);

    /*
     * Mat4 is column-major in this engine — m[col][row] — which matches
     * what GL expects natively. GL_FALSE: no transpose needed.
     */
    s_glUniformMatrix4fv(s_loc_mvp,   1, GL_FALSE, &mvp.m[0][0]);
    s_glUniformMatrix4fv(s_loc_model, 1, GL_FALSE, &model.m[0][0]);

    s_glUniform3f(s_loc_sun_dir,  sun->dir.x, sun->dir.y, sun->dir.z);
    s_glUniform1f(s_loc_ambient,  sun->ambient);
    s_glUniform1f(s_loc_intensity, sun->intensity);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mesh->texture);

    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh->index_count, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);
}