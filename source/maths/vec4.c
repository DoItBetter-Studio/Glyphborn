#include "maths/vec4.h"

Vec4 vec4_create(float x, float y, float z, float w)
{
	return (Vec4) { x, y, z, w };
}

Vec4 vec4_from_vec3(float x, float y, float z)
{
	return (Vec4) { x, y, z, 1.0f };
}

Vec4 vec4_add(Vec4 a, Vec4 b)
{
	return (Vec4) { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

Vec4 vec4_sub(Vec4 a, Vec4 b)
{
	return (Vec4) { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

Vec4 vec4_scale(Vec4 v, float scalar)
{
	return (Vec4) { v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar };
}

Vec4 mat4_mul_vec4(Mat4 m, Vec4 v)
{
	Vec4 result;
	result.x = m.m[0][0] * v.x + m.m[1][0] * v.y + m.m[2][0] * v.z + m.m[3][0] * v.w;
	result.y = m.m[0][1] * v.x + m.m[1][1] * v.y + m.m[2][1] * v.z + m.m[3][1] * v.w;
	result.z = m.m[0][2] * v.x + m.m[1][2] * v.y + m.m[2][2] * v.z + m.m[3][2] * v.w;
	result.w = m.m[0][3] * v.x + m.m[1][3] * v.y + m.m[2][3] * v.z + m.m[3][3] * v.w;
	return result;
}
