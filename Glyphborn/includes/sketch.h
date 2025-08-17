#ifndef SKETCH_H
#define SKETCH_H

#include "render.h"
#include "maths/mat4.h"
#include "maths/vec4.h"
#include "mesh.h"
#include <stdlib.h>

void sketch_draw_wireframe(RenderMesh* mesh, Mat4 model, Mat4 view, Mat4 projection, uint32_t color);

#endif // !SKETCH_H
