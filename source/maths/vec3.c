#include "maths/vec3.h"
#include <math.h>

Vec3 vec3_add(Vec3 a, Vec3 b)
{
	return (Vec3) { a.x + b.x, a.y + b.y, a.z + b.z };
}

Vec3 vec3_sub(Vec3 a, Vec3 b)
{
	return (Vec3) { a.x - b.x, a.y - b.y, a.z - b.z };
}

Vec3 vec3_scale(Vec3 v, float scalar)
{
	return (Vec3) { v.x * scalar, v.y * scalar, v.z * scalar };
}

float vec3_dot(Vec3 a, Vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec3 vec3_cross(Vec3 a, Vec3 b)
{
	return (Vec3) {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	};
}

float vec3_length(Vec3 v)
{
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 vec3_normalize(Vec3 v)
{
	float len = vec3_length(v);
	if (len == 0.0f) return (Vec3) { 0.0f, 0.0f, 0.0f };
	return (Vec3) { v.x / len, v.y / len, v.z / len };
}
