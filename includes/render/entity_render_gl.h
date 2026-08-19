#ifndef ENTITY_RENDER_GL_H
#define ENTITY_RENDER_GL_H

/*
 * entity_render_gl.h - OpenGL 3.3 skinned entity rendering pipeline
 *
 * Draws a SkinnedGPUMesh using a bone palette produced by
 * gb_animation_evaluate(). One draw call per surface per entity.
 *
 * Uniforms updated per draw:
 *   u_mvp        - mat4   model-view-projection
 *   u_bones      - mat4[] bone palette (MAX_BONES entries)
 *   u_sun_dir    - vec3   normalised world-space sun direction
 *   u_ambient    - float  ambient light floor
 *   u_intensity  - float  diffuse intensity scalar
 *   u_albedo     - sampler2D  diffuse texture (unit 0)
 */

#include "render/skinned_gpu_mesh.h"
#include "maths/mat4.h"
#include "models/material.h"
#include "lighting/directional_light.h"

#define ENTITY_MAX_BONES 128

void entity_gl_init(void);
void entity_gl_shutdown(void);
void entity_gl_begin_pass(void);

void entity_gl_upload_material(GbMaterial* material);

void entity_gl_draw(const SkinnedGPUMesh*   mesh,
                    Mat4                    model,
                    Mat4                    view,
                    Mat4                    projection,
                    const Mat4*             bone_palette,
                    uint16_t                bone_count,
                    const DirectionalLight* sun,
                    const GbMaterial*       material);

#endif /* ENTITY_RENDER_GL_H */