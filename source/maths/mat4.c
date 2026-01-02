#include "maths/mat4.h"
#include <math.h>

Mat4 mat4_identity(void)
{
	Mat4 result = { 0 };
	result.m[0][0] = 1.0f;
	result.m[1][1] = 1.0f;
	result.m[2][2] = 1.0f;
	result.m[3][3] = 1.0f;
	return result;
}

Mat4 mat4_translate(Vec3 v)
{
	Mat4 result = mat4_identity();
	result.m[3][0] = v.x;
	result.m[3][1] = v.y;
	result.m[3][2] = v.z;
	return result;
}

Mat4 mat4_scale(Vec3 v)
{
	Mat4 result = mat4_identity();
	result.m[0][0] = v.x;
	result.m[1][1] = v.y;
	result.m[2][2] = v.z;
	return result;
}

Mat4 mat4_rotate_x(float angle)
{
	Mat4 result = mat4_identity();
	float c = cosf(angle);
	float s = sinf(angle);
	result.m[1][1] = c;
	result.m[1][2] = s;
	result.m[2][1] = -s;
	result.m[2][2] = c;
	return result;
}

Mat4 mat4_rotate_y(float angle)
{
	Mat4 result = mat4_identity();
	float c = cosf(angle);
	float s = sinf(angle);
	result.m[0][0] = c;
	result.m[0][2] = -s;
	result.m[2][0] = s;
	result.m[2][2] = c;
	return result;
}

Mat4 mat4_rotate_z(float angle)
{
	Mat4 result = mat4_identity();
	float c = cosf(angle);
	float s = sinf(angle);
	result.m[0][0] = c;
	result.m[0][1] = s;
	result.m[1][0] = -s;
	result.m[1][1] = c;
	return result;
}

Mat4 mat4_multiply(Mat4 a, Mat4 b)
{
	Mat4 result = { 0 };
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result.m[i][j] = a.m[0][j] * b.m[i][0] +
							 a.m[1][j] * b.m[i][1] +
							 a.m[2][j] * b.m[i][2] +
							 a.m[3][j] * b.m[i][3];
		}
	}
	return result;
}

Mat4 mat4_perspective(float fov, float aspect, float near, float far)
{
	Mat4 result = { 0 };
	float f = 1.0f / tanf(fov * 0.5f);

	result.m[0][0] = f / aspect;
	result.m[1][1] = f;
	result.m[2][2] = (far + near) / (near - far);
	result.m[2][3] = -1.0f;
	result.m[3][2] = (2.0f * far * near) / (near - far);
	// result.m[3][3] = 0.0f; // already zeroed

	return result;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up)
{
	Vec3 f = vec3_normalize(vec3_sub(center, eye));     // Forward
	Vec3 s = vec3_normalize(vec3_cross(f, up));         // Right
	Vec3 u = vec3_cross(s, f);                          // Up

	Mat4 result = mat4_identity();

	// Column-major layout: each axis is a column
	result.m[0][0] = s.x;
	result.m[0][1] = s.y;
	result.m[0][2] = s.z;
	result.m[0][3] = 0.0f;

	result.m[1][0] = u.x;
	result.m[1][1] = u.y;
	result.m[1][2] = u.z;
	result.m[1][3] = 0.0f;

	result.m[2][0] = -f.x;
	result.m[2][1] = -f.y;
	result.m[2][2] = -f.z;
	result.m[2][3] = 0.0f;

	result.m[3][0] = -vec3_dot(s, eye);
	result.m[3][1] = -vec3_dot(u, eye);
	result.m[3][2] = vec3_dot(f, eye); // OpenGL-style
	result.m[3][3] = 1.0f;

	return result;
}
