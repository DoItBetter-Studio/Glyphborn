#ifndef SKETCH_H
#define SKETCH_H

#include "render.h"
#include "mesh.h"
#include "maths/mat4.h"
#include <stdlib.h>

void sketch_draw_wireframe(RenderMesh* mesh, Mat4 model, Mat4 view, Mat4 projection, uint32_t color);
void sketch_draw_grid(int size_x, int y_index, int size_z, Mat4 view, Mat4 projection);
#endif // !SKETCH_H
