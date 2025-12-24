#include "maths/conversions.h"

Mat4 mat4_from_quat(Quat q)
{
	q = quat_normalize(q);

	float x = q.x, y = q.y, z = q.z, w = q.w;

	Mat4 m = mat4_identity();

	m.m[0][0] = 1.0f - 2.0f * (y*y + z*z);
	m.m[0][1] = 2.0f * (x*y + z*w);
	m.m[0][2] = 2.0f * (x*z - y*w);

	m.m[1][0] = 2.0f * (x*y - z*w);
	m.m[1][1] = 1.0f - 2.0f * (x*x + z*z);
	m.m[1][2] = 2.0f * (y*z + x*w);

	m.m[2][0] = 2.0f * (x*z + y*w);
	m.m[2][1] = 2.0f * (y*z - x*w);
	m.m[2][2] = 1.0f - 2.0f * (x*x + y*y);

	return m;
}

Mat4 mat4_from_trs(Vec3 t, Quat r, Vec3 s)
{
    Mat4 T = mat4_translate(t);
    Mat4 R = mat4_from_quat(r);
    Mat4 S = mat4_scale(s);

    // local = S * R * T
    return mat4_multiply(T, mat4_multiply(R, S));
}

Mat4 mat4_inverse_affine(Mat4 m)
{
    Mat4 r = mat4_identity();

    // Extract 3x3 rotation/scale
    float a00 = m.m[0][0], a01 = m.m[0][1], a02 = m.m[0][2];
    float a10 = m.m[1][0], a11 = m.m[1][1], a12 = m.m[1][2];
    float a20 = m.m[2][0], a21 = m.m[2][1], a22 = m.m[2][2];

    // Determinant
    float det =
        a00*(a11*a22 - a12*a21) -
        a01*(a10*a22 - a12*a20) +
        a02*(a10*a21 - a11*a20);

    float inv_det = 1.0f / det;

    // Inverse 3x3
    r.m[0][0] =  (a11*a22 - a12*a21) * inv_det;
    r.m[0][1] = -(a01*a22 - a02*a21) * inv_det;
    r.m[0][2] =  (a01*a12 - a02*a11) * inv_det;

    r.m[1][0] = -(a10*a22 - a12*a20) * inv_det;
    r.m[1][1] =  (a00*a22 - a02*a20) * inv_det;
    r.m[1][2] = -(a00*a12 - a02*a10) * inv_det;

    r.m[2][0] =  (a10*a21 - a11*a20) * inv_det;
    r.m[2][1] = -(a00*a21 - a01*a20) * inv_det;
    r.m[2][2] =  (a00*a11 - a01*a10) * inv_det;

    // Inverse translation
    float tx = m.m[3][0];
    float ty = m.m[3][1];
    float tz = m.m[3][2];

    r.m[3][0] = -(r.m[0][0]*tx + r.m[1][0]*ty + r.m[2][0]*tz);
    r.m[3][1] = -(r.m[0][1]*tx + r.m[1][1]*ty + r.m[2][1]*tz);
    r.m[3][2] = -(r.m[0][2]*tx + r.m[1][2]*ty + r.m[2][2]*tz);

    return r;
}
