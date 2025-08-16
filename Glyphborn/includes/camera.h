#ifndef CAMERA_H
#define CAMERA_H

#include "maths/vec3.h"
#include "maths/mat4.h"

typedef struct
{
	Vec3 position;
	Vec3 target;
	Vec3 up;
} Camera;

Mat4 camera_get_view_matrix(Camera* cam);

#endif // !CAMERA_H
