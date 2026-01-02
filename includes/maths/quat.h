#ifndef QUAT_H
#define QUAT_H

#include "vec3.h"

typedef struct
{
	float w;
	float x;
	float y;
	float z;
} Quat;

Quat quat_identity(void);
Quat quat_from_axis_angle(Vec3 axis, float angle);
Quat quat_multiply(Quat a, Quat b);
Quat quat_normalize(Quat q);

#endif // !QUAT_H
