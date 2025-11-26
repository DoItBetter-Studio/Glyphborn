#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"

typedef struct
{
	float m[4][4];
} Mat4;

Mat4 mat4_identity(void);
Mat4 mat4_translate(Vec3 v);
Mat4 mat4_scale(Vec3 v);
Mat4 mat4_rotate_x(float angle);
Mat4 mat4_rotate_y(float angle);
Mat4 mat4_rotate_z(float angle);
Mat4 mat4_multiply(Mat4 a, Mat4 b);

Mat4 mat4_perspective(float fov, float aspect, float near, float far);
Mat4 mat4_orthographic(float left, float right, float top, float bottom, float near, float far);
Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up);
Mat4 orbit_camera(float radius, float yaw, float pitch);
#endif // !MAT4_H
