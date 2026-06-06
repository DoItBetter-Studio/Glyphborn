#ifndef WORLD_RENDER_GL_H
#define WORLD_RENDER_GL_H

/*
 * world_render_gl.h - OpenGL 3.3 world rendering pipeline
 *
 * Owns the shader program used to draw GPUMesh chunks.
 * Must be initialised once after the GL context is ready, and shut down
 * before the context is destroyed.
 *
 * Uniforms updated per frame:
 *   u_mvp        - mat4  model-view-projection
 *   u_model      - mat4  model matrix (for normal transform)
 *   u_sun_dir    - vec3  normalised world-space sun direction
 *   u_ambient    - float ambient light floor
 *   u_intensity  - float diffuse intensity scalar
 *
 * Texture unit 0 is the chunk atlas (bound by the caller per draw).
 */

#include "gpu_mesh.h"
#include "maths/mat4.h"
#include "lighting/directional_light.h"

void world_gl_init(void);
void world_gl_shutdown(void);

void world_gl_begin_pass(void);

void world_gl_draw(const GPUMesh*         mesh,
                   Mat4                   model,
                   Mat4                   view,
                   Mat4                   projection,
                   const DirectionalLight* sun);

#endif /* WORLD_RENDER_GL_H */