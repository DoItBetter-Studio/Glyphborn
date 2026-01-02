#ifndef VEC4_H
#define VEC4_H

#include "mat4.h"

typedef struct
{
	float x;
	float y;
	float z;
	float w;
} Vec4;

Vec4 vec4_create(float x, float y, float z, float w);
Vec4 vec4_from_vec3(float x, float y, float z);

Vec4 vec4_add(Vec4 a, Vec4 b);
Vec4 vec4_sub(Vec4 a, Vec4 b);
Vec4 vec4_scale(Vec4 v, float scalar);
Vec4 mat4_mul_vec4(Mat4 m, Vec4 v);

#endif // !VEC4_H
