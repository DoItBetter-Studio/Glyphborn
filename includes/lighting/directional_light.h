#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "maths/vec3.h"

typedef struct DirectionalLight {
    Vec3 dir;
    float ambient;
    float intensity;
} DirectionalLight;

#endif // !DIRECTIONAL_LIGHT_H