// Inside camera.c
#include "camera.h"
#include <math.h>

void camera_cycle_facing(CameraFacing* current_facing, int direction)
{
    // C modulo handles negatives poorly, so we add 4 to guarantee a safe positive wrap
    int next_facing = ((int)(*current_facing) + direction + 4) % 4;
    *current_facing = (CameraFacing)next_facing;
}

static float lerp_angle(float current, float target, float factor)
{
	// Handle wrapping issues so the camera doesn't spin 270 degrees 
    // backward when moving from 0 to 275 degrees.
	float difference = target - current;
    while (difference < -3.14159265f) difference += 2.0f * 3.14159265f;
    while (difference >  3.14159265f) difference -= 2.0f * 3.14159265f;
    
    return current + difference * factor;
}

void camera_update(Camera* cam, Vec3 target_focus, CameraFacing facing, float delta_time)
{
    float distance = 14.14f;
    float height = 10.0f; 

    cam->pitch = 0.615f;
    // cam->pitch = 0.15f;

    // FIXED: Corrected angles so East matches +X and West matches -X 
    // when processed through sin() and cos()
    switch (facing)
    {
        case CAM_FACE_NORTH:
            cam->target_yaw = 0.0f;                       // Position: (0, height, distance)
            break;
            
        case CAM_FACE_EAST:
            cam->target_yaw = 3.14159265f / 2.0f;         // Position: (distance, height, 0)
            break;
            
        case CAM_FACE_SOUTH:
            cam->target_yaw = 3.14159265f;                // Position: (0, height, -distance)
            break;
            
        case CAM_FACE_WEST:
            cam->target_yaw = -3.14159265f / 2.0f;        // Position: (-distance, height, 0)
            break;
    }

    // Smoothly interpolate current yaw toward target yaw
    float lerp_speed = 5.0f; 
#if __YGGDRASIL__
    // Bare-metal fallback: Use a simple linear approximation for frame-rate dampening
    float factor = lerp_speed * delta_time;
    if (factor > 1.0f) factor = 1.0f;
#else
    // Standard PC platforms use the math library's exponential decay function
    float factor = 1.0f - expf(-lerp_speed * delta_time);
#endif
    cam->current_yaw = lerp_angle(cam->current_yaw, cam->target_yaw, factor);

    // Calculate position via standard orbit math
    cam->position.x = target_focus.x - distance * sinf(cam->current_yaw);
    cam->position.y = target_focus.y + height;
    cam->position.z = target_focus.z + distance * cosf(cam->current_yaw);
}

Mat4 camera_get_view_matrix(Camera* cam)
{
    // 1. Start with an identity matrix
    Mat4 view;
    mat4_identity(&view);

    // 2. Create the negative position translation
    Mat4 translation = mat4_translate((Vec3){-cam->position.x, -cam->position.y, -cam->position.z});

    // 3. Create the Y-axis rotation (Yaw) based on where we are facing
    Mat4 rotation_y = mat4_rotate_y(cam->current_yaw);

    // 4. Create the X-axis rotation (Pitch) for the overhead 3D view tilt
    Mat4 rotation_x = mat4_rotate_x(cam->pitch);

    // FIX: Multiply right-to-left: RotationX * RotationY * Translation
    view = mat4_multiply(rotation_x, rotation_y);
    view = mat4_multiply(view, translation);

    return view;
}