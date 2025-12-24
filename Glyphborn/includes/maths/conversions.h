#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include "mat4.h"
#include "quat.h"
#include "vec2.h"
#include "vec3.h"
#include "vec4.h"

Mat4 mat4_from_quat(Quat q);
Mat4 mat4_from_trs(Vec3 t, Quat r, Vec3 s);
Mat4 mat4_inverse_affine(Mat4 m);

#endif // !CONVERSIONS_H