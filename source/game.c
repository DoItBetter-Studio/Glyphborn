#ifdef __linux__
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L // 199309L explicitly unlocks clock_gettime
#endif
#include <time.h>
#endif

#include "game.h"
#include "input.h"
#include "camera.h"
#include "test_cube.h"
#include "audio.h"
#include "ui_skin.h"
#include "ui.h"
#include "sketch.h"
#include "achievements.h"
#include "world/world.h"
#include "lighting/directional_light.h"
#include "generated/Audio.h"
#include "generated/Geometry.h"
#include "generated/Collision.h"
#include "generated/Tileset_Regional.h"
#include "generated/Tileset_Local.h"
#include "generated/Tileset_Interior.h"

#include <stdio.h>
#include <math.h>

Camera main_camera = {0};
Mat4 view;
Mat4 projection;

static CameraFacing current_facing = CAM_FACE_NORTH;

static void draw_test_pattern_ui(void)
{
	for (int y = 200; y < FB_HEIGHT - 16; ++y)
	{
		for (int x = 16; x < FB_WIDTH - 16; ++x)
		{
			bool is_light = ((x / 16) + (y / 16)) % 2 == 0;
			uint32_t color = is_light ? 0x88FFFFFF : 0x88000000; // semi-transparent white/black
			framebuffer_ui[y * FB_WIDTH + x] = color;
		}
	}
}

World world;
DirectionalLight sun;

void game_init(void)
{
	input_init();
	//achievements_init();

	Geometry_init();
	Collision_init();
	Tileset_Regional_init();
	Tileset_Local_init();
	Tileset_Interior_init();

	ui_skins_init();
	ui_set_skin(SKIN_GLYPHBORN);

	main_camera.position = (Vec3){0.0f, 10.0f, -10.0f};
	main_camera.target = (Vec3){main_camera.position.x, 0, main_camera.position.z + 10.0f};
	main_camera.up = (Vec3){0.0f, 1.0f, 0.0f};

	view = camera_get_view_matrix(&main_camera);

	projection = mat4_perspective(3.14159f / 4.0f, (float)FB_WIDTH / (float)FB_HEIGHT, 0.1f, 100.0f);

	world_init(&world, main_camera.position.x, main_camera.position.z);
	world_update(&world, main_camera.position.x, main_camera.position.z);

	sun = (DirectionalLight){ .dir = vec3_normalize((Vec3){-0.4f, -1.0f, 0.2f}), .ambient = 0.35f, .intensity = 0.75f };

	audio_play_music(MUSIC_MUSIC_THE_LONGEST_JOURNEY, true);
}

static bool activate = false;
int nav_dx = 0, nav_dy = 0;
static bool showUI = false;
static bool showUVs = false;
static float sun_angle = 0.0f;
static float season = 0.0f;

void game_update(float delta_time)
{
	input_update();
	// achievements_update();

	// These come from your input layer
	nav_dx = 0;
	nav_dy = 0;
	if (input_button_down(BUTTON_UP))
		nav_dy += -1;
	if (input_button_down(BUTTON_DOWN))
		nav_dy += 1;
	activate = input_button_down(BUTTON_A);

	if (input_button_down(BUTTON_SELECT))
	{
		showUVs = !showUVs;
	}

	// 1. Handle snappy rotation switching (Example: SELECT button)
    if (input_button_down(BUTTON_LEFT_BUMPER))
    {
        camera_cycle_facing(&current_facing, -1);
    }
	else if (input_button_down(BUTTON_RIGHT_BUMPER))
	{
		camera_cycle_facing(&current_facing, 1);
	}

    // 2. Track an anchor target point in the world instead of moving camera coordinates directly
    static Vec3 camera_focus = {0.0f, 0.0f, 0.0f};
    float move_speed = 5.0f;
    Vec3 move_dir = {0};

    if (input_get_button(BUTTON_UP))    move_dir.z -= 1.0f; // Screen Forward (-Z in view space)
    if (input_get_button(BUTTON_DOWN))  move_dir.z += 1.0f; // Screen Backward (+Z in view space)
    if (input_get_button(BUTTON_LEFT))  move_dir.x -= 1.0f; // Screen Left (-X)
    if (input_get_button(BUTTON_RIGHT)) move_dir.x += 1.0f; // Screen Right (+X)

    // Translate screen movement directly into world coordinates based on camera orientation
    switch (current_facing)
    {
        case CAM_FACE_NORTH:
            camera_focus.x += move_dir.x * move_speed * delta_time;
            camera_focus.z += move_dir.z * move_speed * delta_time;
            break;
        case CAM_FACE_EAST:
            camera_focus.z += move_dir.x * move_speed * delta_time;
            camera_focus.x -= move_dir.z * move_speed * delta_time;
            break;
        case CAM_FACE_SOUTH:
            camera_focus.x -= move_dir.x * move_speed * delta_time;
            camera_focus.z -= move_dir.z * move_speed * delta_time;
            break;
        case CAM_FACE_WEST:
            camera_focus.z -= move_dir.x * move_speed * delta_time;
            camera_focus.x += move_dir.z * move_speed * delta_time;
            break;
    }

    // 3. Recompute fixed positions and update the view matrix
    camera_update(&main_camera, camera_focus, current_facing, delta_time);
    view = camera_get_view_matrix(&main_camera);

	sketch_show_uvs(showUVs);

	// Day/night cycle
	sun_angle += delta_time * 0.25f;
	if (sun_angle > 6.28318f)
		sun_angle -= 6.28318f;

	// Season cycle (Will eventually become it's own system)
	// This makes a full seasonal cycle every ~25 seconds at 0.25 speed
	// season += delta_time * 0.01f;
	// if (season > 1.0f) season -= 1.0f;

	// Calculate sun direction with seasonal variation
	float y = sinf(sun_angle);
	float horizontal = cosf(sun_angle);

	// Season affects sun height (0 = winter/low, 1 = summer/high)
	float season_height = 0.6f + season * 0.4f; // Range: 0.6 to 1.0
	y *= season_height;

	// Optional: tilt the sun's path for more realistic seasons
	float tilt = season * 0.3f;
	float z = horizontal * sinf(tilt);
	float x = horizontal * cosf(tilt);

	sun.dir = vec3_normalize((Vec3){x, y, z});

	// Optional: adjust ambient based on season
	sun.ambient = 0.25f + season * 0.15f; // Brighter in summer

	world_update(&world, camera_focus.x, camera_focus.z);
}

static void draw_cross()
{
	Vec3 zero = {
		0, 0, 0
	};

	Vec3 right = {
		3, 0, 0
	};

	Vec3 up = {
		0, 3, 0
	};

	Vec3 forward = {
		0, 0, 3
	};

	sketch_draw_line_3d(zero, right, view, projection, 0xFFFF0000);
	sketch_draw_line_3d(zero, up, view, projection, 0xFF00FF00);
	sketch_draw_line_3d(zero, forward, view, projection, 0xFF0000FF);
}

void game_render(void)
{
	sketch_clear(0xFF000000);

	view = camera_get_view_matrix(&main_camera);

	world_render(&world, view, projection);

	draw_cross();
}

void game_render_ui(void)
{
    ui_begin_frame(0, 0, false, activate);

    if (nav_dx || nav_dy)
    {
        g_ui.nav_mode = true;
        g_ui.focused_id += nav_dy;
        if (g_ui.focused_id < 1)
            g_ui.focused_id = 1;
    }

    if (ui_button(20, 20, 128, 48, "Hello World!", 0xFF606060)) {}

    if (showUI)
    {
        draw_test_pattern_ui();
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%f, %f, %f", main_camera.position.x, main_camera.position.y, main_camera.position.z);

    ui_draw_text_colored(200, 10, (const char *)buffer, 0xFFFFFFFF);

    static char* facing_dir;

    switch (current_facing)
    {
        case CAM_FACE_NORTH: facing_dir = "north"; break;
        case CAM_FACE_SOUTH: facing_dir = "south"; break;
        case CAM_FACE_EAST: facing_dir = "east"; break;
        case CAM_FACE_WEST: facing_dir = "west"; break;
    }

    snprintf(buffer, sizeof(buffer), "facing: %s", facing_dir);

    ui_draw_text_colored(500, 10, (const char *)buffer, 0xFFFFFFFF);

    char* dot = ".";
    int dot_w = ui_text_width(dot);
    int dot_h = ui_text_height(dot, 8);

    int text_x = (FB_WIDTH - dot_w) / 2;
    int text_y = (FB_HEIGHT - dot_h) / 2;

    ui_draw_text_colored(text_x, text_y, dot, 0xFFFF00FF);
    ui_end_frame();
}

void game_shutdown(void)
{
	achievements_shutdown();
	world_free(&world);
}
