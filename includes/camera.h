#ifndef CAMERA_H
#define CAMERA_H

#include "maths/vec3.h"
#include "maths/mat4.h"

typedef enum {
    CAM_FACE_NORTH, // Looking down -Z
    CAM_FACE_EAST,  // Looking down +X
    CAM_FACE_SOUTH, // Looking down +Z
    CAM_FACE_WEST   // Looking down -X
} CameraFacing;

typedef struct
{
	Vec3 position;
	Vec3 target;
	Vec3 up;
	float current_yaw;
	float target_yaw;
	float pitch;
} Camera;

void camera_cycle_facing(CameraFacing* current_facing, int direction);
void camera_update(Camera* cam, Vec3 target_focus, CameraFacing facing, float delta_time);
Mat4 camera_get_view_matrix(Camera* cam);

#endif // !CAMERA_H
