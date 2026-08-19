#include "maths/quat.h"
#include <math.h>

Quat quat_identity(void)
{
	return (Quat) { 0.0f, 0.0f, 0.0f, 1.0f };
}

Quat quat_from_axis_angle(Vec3 axis, float angle)
{
	Quat q;
	float half_angle = angle * 0.5f;
	float s = sinf(half_angle);
	Vec3 norm_axis = vec3_normalize(axis);
	q.w = cosf(half_angle);
	q.x = norm_axis.x * s;
	q.y = norm_axis.y * s;
	q.z = norm_axis.z * s;
	return q;
}

Quat quat_multiply(Quat a, Quat b)
{
	Quat result;
	result.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	result.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	result.y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
	result.z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
	return result;
}

Quat quat_normalize(Quat q)
{
	float len = sqrtf(q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z);
	if (len == 0.0f) return quat_identity();
	return (Quat) { q.x / len, q.y / len, q.z / len, q.w / len };
}

Vec3 quat_rotate(Quat q, Vec3 v)
{
    // q * v * q^-1 using quaternion multiplication
    Vec3 u = {q.x, q.y, q.z};
    float s = q.w;
    
    Vec3 t1 = vec3_scale(u, 2.0f * vec3_dot(u, v));
    Vec3 t2 = vec3_scale(v, s * s - vec3_dot(u, u));
    Vec3 t3 = vec3_scale(vec3_cross(u, v), 2.0f * s);
    
    return vec3_add(vec3_add(t1, t2), t3);
}