/*
 * entity_render_gl.c - OpenGL 3.3 skinned entity rendering pipeline
 *
 * Vertex shader blends vertex positions and normals by up to 4 bone
 * influences using the palette produced by gb_animation_evaluate().
 * Fragment shader samples the diffuse texture and applies Lambert lighting.
 *
 * Mat4 is column-major (m[col][row]) — matches what GL expects natively,
 * so GL_FALSE is passed to glUniformMatrix4fv (no transpose needed).
 */

#include "render/entity_render_gl.h"
#include "render/gl_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* GL_RGBA not in gl_loader.h — all others are */
#define GL_RGBA                  0x1908

/* -------------------------------------------------------------------------
 * Shader source
 * ------------------------------------------------------------------------- */
static const char* s_vert =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "layout(location = 1) in vec3 a_normal;\n"
    "layout(location = 2) in vec2 a_uv;\n"
    "layout(location = 3) in vec4 a_bone_idx;\n"
    "layout(location = 4) in vec4 a_bone_wt;\n"
    "\n"
    "#define MAX_BONES 128\n"
    "uniform mat4 u_mvp;\n"
    "uniform mat4 u_bones[MAX_BONES];\n"
    "\n"
    "out vec3 v_normal;\n"
    "out vec2 v_uv;\n"
    "\n"
    "void main() {\n"
    "    mat4 skin =\n"
    "        u_bones[int(a_bone_idx.x)] * a_bone_wt.x +\n"
    "        u_bones[int(a_bone_idx.y)] * a_bone_wt.y +\n"
    "        u_bones[int(a_bone_idx.z)] * a_bone_wt.z +\n"
    "        u_bones[int(a_bone_idx.w)] * a_bone_wt.w;\n"
    "\n"
    "    vec4 skinned_pos    = skin * vec4(a_pos,    1.0);\n"
    "    vec3 skinned_normal = mat3(skin) * a_normal;\n"
    "\n"
    "    gl_Position = u_mvp * skinned_pos;\n"
    "    v_normal    = skinned_normal;\n"
    "    v_uv        = a_uv;\n"
    "}\n";

static const char* s_frag =
    "#version 330 core\n"
    "in  vec3 v_normal;\n"
    "in  vec2 v_uv;\n"
    "out vec4 frag_color;\n"
    "\n"
    "uniform sampler2D u_albedo;\n"
    "uniform vec3      u_sun_dir;\n"
    "uniform float     u_ambient;\n"
    "uniform float     u_intensity;\n"
    "\n"
    "void main() {\n"
    "    float ndotl = max(dot(normalize(v_normal), u_sun_dir), 0.0);\n"
    "    float light = clamp(u_ambient + ndotl * u_intensity, 0.0, 1.0);\n"
    "    vec4  texel = texture(u_albedo, v_uv);\n"
    "    //frag_color  = vec4(texel.rgb * light, texel.a);\n"
    "    frag_color = texture(u_albedo, v_uv);"
    "}\n";

/* -------------------------------------------------------------------------
 * Pipeline state
 * ------------------------------------------------------------------------- */
static GLuint s_program     = 0;
static GLint  s_loc_mvp     = -1;
static GLint  s_loc_bones   = -1;
static GLint  s_loc_albedo  = -1;
static GLint  s_loc_sun_dir = -1;
static GLint  s_loc_ambient = -1;
static GLint  s_loc_intens  = -1;

/* 1x1 white fallback texture — used when material is NULL */
static GLuint s_fallback_tex = 0;

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
            fprintf(stderr, "entity_render_gl: shader error:\n%s\n", log);
            free(log);
        }
    }
    return shader;
}

static void build_fallback_texture(void)
{
    uint32_t white = 0xFFFFFFFFu;

    glGenTextures(1, &s_fallback_tex);
    glBindTexture(GL_TEXTURE_2D, s_fallback_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, &white);
    glBindTexture(GL_TEXTURE_2D, 0);
}

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */
void entity_gl_init(void)
{
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
            fprintf(stderr, "entity_render_gl: link error:\n%s\n", log);
            free(log);
        }
    }

    glDeleteShader(vert);
    glDeleteShader(frag);

    glUseProgram(s_program);
    s_loc_mvp     = glGetUniformLocation(s_program, "u_mvp");
    s_loc_bones   = glGetUniformLocation(s_program, "u_bones");
    s_loc_albedo  = glGetUniformLocation(s_program, "u_albedo");
    s_loc_sun_dir = glGetUniformLocation(s_program, "u_sun_dir");
    s_loc_ambient = glGetUniformLocation(s_program, "u_ambient");
    s_loc_intens  = glGetUniformLocation(s_program, "u_intensity");
    glUniform1i(s_loc_albedo, 0); /* texture unit 0 — set once, never changes */
    glUseProgram(0);

    build_fallback_texture();
}

void entity_gl_shutdown(void)
{
    if (s_fallback_tex) { glDeleteTextures(1, &s_fallback_tex); s_fallback_tex = 0; }
    if (s_program)      { glDeleteProgram(s_program); s_program = 0; }
}

void entity_gl_begin_pass(void)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

void entity_gl_upload_material(GbMaterial* mat)
{
    if (!mat || mat->gpu_handle != 0) return;

    glGenTextures(1, &mat->gpu_handle);
    glBindTexture(GL_TEXTURE_2D, mat->gpu_handle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                 (GLsizei)mat->width, (GLsizei)mat->height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, mat->pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void entity_gl_draw(const SkinnedGPUMesh*   mesh,
                    Mat4                    model,
                    Mat4                    view,
                    Mat4                    projection,
                    const Mat4*             bone_palette,
                    uint16_t                bone_count,
                    const DirectionalLight* sun,
                    const GbMaterial*       material)
{
    if (!mesh || !mesh->vao || !mesh->index_count) return;
    if (!s_program || !glUniformMatrix4fv)          return;

    Mat4 mv  = mat4_multiply(view, model);
    Mat4 mvp = mat4_multiply(projection, mv);

    glUseProgram(s_program);

    glUniformMatrix4fv(s_loc_mvp, 1, GL_FALSE, &mvp.m[0][0]);

    uint16_t count = bone_count < ENTITY_MAX_BONES ? bone_count : ENTITY_MAX_BONES;
    glUniformMatrix4fv(s_loc_bones, count, GL_FALSE, &bone_palette[0].m[0][0]);

    glUniform3f(s_loc_sun_dir, sun->dir.x, sun->dir.y, sun->dir.z);
    glUniform1f(s_loc_ambient, sun->ambient);
    glUniform1f(s_loc_intens,  sun->intensity);

    /* Bind material texture — fall back to white if none provided */
    GLuint tex = (material && material->gpu_handle)
               ? material->gpu_handle
               : s_fallback_tex;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)mesh->index_count, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}