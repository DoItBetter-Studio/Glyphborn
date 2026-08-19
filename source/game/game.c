#ifdef __linux__
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L // 199309L explicitly unlocks clock_gettime
#endif
#include <time.h>
#endif

#include "game/game.h"
#include "input/input.h"
#include "render/camera.h"
#include "render/test_cube.h"
#include "audio/audio.h"
#include "ui/ui_skin.h"
#include "ui/ui.h"
#include "render/sketch.h"
#include "achievements/achievements.h"
#include "world/world.h"
#include "lighting/directional_light.h"
#include "generated/Audio.h"
#include "generated/Geometry.h"
#include "generated/Collision.h"
#include "generated/Tileset_Regional.h"
#include "generated/Tileset_Local.h"
#include "generated/Tileset_Interior.h"
#include "generated/Animations.h"
#include "generated/Meshs.h"
#include "generated/Skeletons.h"
#include "generated/Materials.h"
#include "generated/DialogueTable.h"
#include "generated/Locales.h"
#include "generated/LocaleBindings.h"
#include "dialogue/locale.h"

#include "models/skeleton.h"
#include "models/mesh.h"
#include "models/animation.h"

#include "render/entity_render_gl.h"
#include "render/skinned_gpu_mesh.h"

#include "dialogue/dialogue.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

Camera main_camera = {0};
Mat4 view;
Mat4 projection;

static CameraFacing current_facing = CAM_FACE_NORTH;

static GbSkeleton*  test_skel  = NULL;
static GbMesh*      test_mesh  = NULL;
static GbAnimation* test_anim  = NULL;
static GbMaterial*   test_material = NULL;
static Mat4*        test_palette = NULL;
static float        test_frame   = 0.0f;
static float 		test_fps   = 60.0f;   // or 30.0f
static SkinnedGPUMesh test_gpu_mesh = {0};

static void draw_test_pattern_ui(void)
{
	for (int32_t y = 200; y < FB_HEIGHT - 16; ++y)
	{
		for (int32_t x = 16; x < FB_WIDTH - 16; ++x)
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
	//achievements_init();

	Geometry_init();
	Collision_init();
	Tileset_Regional_init();
	Tileset_Local_init();
	Tileset_Interior_init();
	Meshs_init();
	Animations_init();
	Skeletons_init();
	Materials_init();
	DialogueTable_init();
	Locales_init();

	test_skel    = gb_skeleton_load(g_Skeletons[0].data);
	test_mesh    = gb_mesh_load(g_Meshs[0].data);
	test_anim    = gb_animation_load(0);
	test_material = gb_material_load(g_Materials[0].data);

	entity_gl_init();

	if (test_mesh && test_mesh->surface_count > 0)
	{
		if (!skinned_gpu_mesh_upload(&test_gpu_mesh, &test_mesh->surfaces[0]))
		{
			fprintf(stderr, "Failed to upload skinned mesh: surface 0 verts=%d faces=%d\n",
			        test_mesh->surfaces[0].vert_count,
			        test_mesh->surfaces[0].face_count);
		}
	}

	if (test_skel)
		test_palette = malloc(sizeof(Mat4) * test_skel->bone_count);

	entity_gl_upload_material(test_material);

	dialogue_load();
	locale_set_active(0);

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
int32_t nav_dx = 0, nav_dy = 0;
static bool showUI = false;
static bool showUVs = false;
static float sun_angle = 1.5708f;
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

	if (test_anim && test_skel && test_palette)
	{
		const GbClip* clip = &test_anim->clips[0];

		test_frame += delta_time * test_fps;

		if (clip->loop)
		{
			while (test_frame >= clip->frame_count)
				test_frame -= clip->frame_count;
		}
		else if (test_frame >= clip->frame_count)
		{
			test_frame = (float)(clip->frame_count - 1);
		}

		gb_animation_evaluate(
			clip,
			test_frame,
			test_skel->bones,
			test_skel->bone_count,
			test_palette);
	}
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

	Mat4 mat = {0};
	mat4_identity(&mat);

	if (test_gpu_mesh.vao && test_gpu_mesh.index_count > 0)
	{
		Mat4 mat;
		mat4_identity(&mat);
		entity_gl_begin_pass();
		entity_gl_draw(&test_gpu_mesh, mat, view, projection,
			test_palette, test_skel->bone_count, &sun, test_material);
	}

	draw_cross();
}

void game_render_ui(void)
{
    ui_begin_frame(input_get_mouse_x(), input_get_mouse_y(), input_mouse_button_down(), activate);

    if (nav_dx || nav_dy)
    {
        g_ui.nav_mode = true;
        g_ui.focused_id += nav_dy;
        if (g_ui.focused_id < 1)
            g_ui.focused_id = 1;
    }

    if (ui_button(20, 20, 128, 48, locale_get_string_terminated(UI_MENU_START_GAME), 0xFF606060)) {}

	if (ui_button(20, 72, 128, 48, locale_get_string_terminated(LOCALES_EN_US), 0xFF121212))
	{
		locale_set_active(0);
	}

	if (ui_button(20, 124, 128, 48, locale_get_string_terminated(LOCALES_PT_BR), 0xFF121212))
	{
		locale_set_active(1);
	}

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
    int32_t dot_w = ui_text_width(dot);
    int32_t dot_h = ui_text_height(dot, 8);

    int32_t text_x = (FB_WIDTH - dot_w) / 2;
    int32_t text_y = (FB_HEIGHT - dot_h) / 2;

    ui_draw_text_colored(text_x, text_y, dot, 0xFFFF00FF);
    ui_end_frame();
}

void game_shutdown(void)
{
	locale_free_active();
	dialogue_free();
	achievements_shutdown();
	world_free(&world);
	free(test_palette);
	gb_skeleton_free(test_skel);
	gb_mesh_free(test_mesh);
	gb_animation_free(test_anim);
	entity_gl_shutdown();
	skinned_gpu_mesh_free(&test_gpu_mesh);
}
