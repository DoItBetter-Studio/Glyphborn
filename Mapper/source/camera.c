#include "camera.h"

Mat4 camera_get_view_matrix(Camera* cam)
{
	return mat4_look_at(cam->position, cam->target, cam->up);
}