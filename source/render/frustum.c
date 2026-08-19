#include "render/frustum.h"
#include <math.h>
#include <stdint.h>

static void normalize_plane(Plane* plane)
{
    float mag = sqrtf(
        plane->a * plane->a +
        plane->b * plane->b +
        plane->c * plane->c
    );

    if (mag == 0.0f)
        return;

    plane->a /= mag;
    plane->b /= mag;
    plane->c /= mag;
    plane->d /= mag;
}

void frustum_extract(Frustum* frustum, Mat4 m)
{
    // LEFT
    frustum->planes[0].a = m.m[0][3] + m.m[0][0];
    frustum->planes[0].b = m.m[1][3] + m.m[1][0];
    frustum->planes[0].c = m.m[2][3] + m.m[2][0];
    frustum->planes[0].d = m.m[3][3] + m.m[3][0];

    // RIGHT
    frustum->planes[1].a = m.m[0][3] - m.m[0][0];
    frustum->planes[1].b = m.m[1][3] - m.m[1][0];
    frustum->planes[1].c = m.m[2][3] - m.m[2][0];
    frustum->planes[1].d = m.m[3][3] - m.m[3][0];

    // BOTTOM
    frustum->planes[2].a = m.m[0][3] + m.m[0][1];
    frustum->planes[2].b = m.m[1][3] + m.m[1][1];
    frustum->planes[2].c = m.m[2][3] + m.m[2][1];
    frustum->planes[2].d = m.m[3][3] + m.m[3][1];

    // TOP
    frustum->planes[3].a = m.m[0][3] - m.m[0][1];
    frustum->planes[3].b = m.m[1][3] - m.m[1][1];
    frustum->planes[3].c = m.m[2][3] - m.m[2][1];
    frustum->planes[3].d = m.m[3][3] - m.m[3][1];

    // NEAR
    frustum->planes[4].a = m.m[0][3] + m.m[0][2];
    frustum->planes[4].b = m.m[1][3] + m.m[1][2];
    frustum->planes[4].c = m.m[2][3] + m.m[2][2];
    frustum->planes[4].d = m.m[3][3] + m.m[3][2];

    // FAR
    frustum->planes[5].a = m.m[0][3] - m.m[0][2];
    frustum->planes[5].b = m.m[1][3] - m.m[1][2];
    frustum->planes[5].c = m.m[2][3] - m.m[2][2];
    frustum->planes[5].d = m.m[3][3] - m.m[3][2];

    for (int32_t i = 0; i < 6; i++)
        normalize_plane(&frustum->planes[i]);
}

bool frustum_aabb_visible(const Frustum* frustum, Vec3 min, Vec3 max)
{
    for (int32_t i = 0; i < 6; i++)
    {
        const Plane* p = &frustum->planes[i];

        Vec3 positive = {
            (p->a >= 0.0f) ? max.x : min.x,
            (p->b >= 0.0f) ? max.y : min.y,
            (p->c >= 0.0f) ? max.z : min.z
        };

        float dist =
            p->a * positive.x +
            p->b * positive.y +
            p->c * positive.z +
            p->d;

        if (dist < 0.0f)
        {
            return false;
        }
    }

    return true;
}