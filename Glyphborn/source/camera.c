#include "camera.h"

Mat4 camera_get_view_matrix(const Camera* cam)
{
	return mat4_look_at(cam->position, cam->target, cam->up);
}