#ifndef FRUSTUM_H
#define FRUSTUM_H

#include <stdbool.h>
#include "maths/mat4.h"
#include "maths/vec3.h"

typedef struct {
    float a, b, c, d;
} Plane;

typedef struct {
    Plane planes[6]; // Left, Right, Top, Bottom, Near, Far
} Frustum;

void frustum_extract(Frustum* frustum, Mat4 view_projection);

bool frustum_aabb_visible(const Frustum* frustum, Vec3 min, Vec3 max);

#endif // !FRUSTUM_H