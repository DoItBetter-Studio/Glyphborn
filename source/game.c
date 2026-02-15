#include "game.h"
#include "input.h"
#include "camera.h"
#include "test_cube.h"
#include "audio.h"
#include "audio/player_jump.h"
#include "ui_skin.h"
#include "ui.h"
#include "sketch.h"
#include "achievements.h"
#include "world/world.h"
#include "lighting/directional_light.h"

#include <stdio.h>
#include <math.h>

Camera main_camera = { 0 };
Mat4 view;
Mat4 projection;

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
	achievements_init();

	ui_set_skin(SKIN_GLYPHBORN);

	main_camera.position = (Vec3){ 16.0f, 1.0f, -16.0f };
	main_camera.target   = (Vec3){ 16.0f, 0.0f,  16.0f };
	main_camera.up = (Vec3){ 0.0f, 1.0f, 0.0f };

	view = camera_get_view_matrix(&main_camera);
	projection = mat4_perspective(3.14159f / 4.0f, (float)FB_WIDTH / (float)FB_HEIGHT, 0.1f, 100.0f);

	world_init(&world, main_camera.position.x, main_camera.position.z);

	sun = (DirectionalLight){
		.dir = vec3_normalize((Vec3){ -0.4f, -1.0f, 0.2f }),
		.ambient = 0.35f,
		.intensity = 0.75f
	};
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
	achievements_update();

	// These come from your input layer
	nav_dx = 0;
	nav_dy = 0;
	if (input_button_down(BUTTON_UP))   nav_dy += -1;
	if (input_button_down(BUTTON_DOWN)) nav_dy += 1;
	activate = input_button_down(BUTTON_A);

	if (input_button_down(BUTTON_B))
	{
		// audio_play_sound(player_jump, PLAYER_JUMP_LEN);
	}

	if (input_button_down(BUTTON_START))
	{
		showUI = !showUI;
	}

	if (input_get_button(BUTTON_UP)) main_camera.position.z += 1.0f * delta_time;
	if (input_get_button(BUTTON_DOWN)) main_camera.position.z -= 1.0f * delta_time;
	if (input_get_button(BUTTON_LEFT)) main_camera.position.x -= 1.0f * delta_time;
	if (input_get_button(BUTTON_RIGHT)) main_camera.position.x += 1.0f * delta_time;

	if (input_button_down(BUTTON_SELECT))
	{
		showUVs = !showUVs;
	}

	sketch_show_uvs(showUVs);

	view = camera_get_view_matrix(&main_camera);

	// Day/night cycle
	sun_angle += delta_time * 0.25f;
	if (sun_angle > 6.28318f) sun_angle -= 6.28318f;

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
	float x = horizontal * sinf(tilt);
	float z = horizontal * cosf(tilt);

	sun.dir = vec3_normalize((Vec3) { x, y, z});

	// Optional: adjust ambient based on season
	sun.ambient = 0.25f + season * 0.15f; // Brighter in summer

	world_update(&world, main_camera.position.x, main_camera.position.z);
}

void game_render(void)
{    
	sketch_clear(0xFF000000);

    view = camera_get_view_matrix(&main_camera);

	world_render(&world, view, projection);
}

void game_render_ui(void)
{
	ui_begin_frame(0, 0, false, activate);

	if (nav_dx || nav_dy)
	{
		g_ui.nav_mode = true;
		g_ui.focused_id += nav_dy;
		if (g_ui.focused_id < 1) g_ui.focused_id = 1;
	}

	if (ui_button(16, 16, 100, 30, "Jump", 0xFF000000))
	{
		audio_play_sound(player_jump, PLAYER_JUMP_LEN);
	}

	if (showUI)
	{
		draw_test_pattern_ui();
	}

	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%f, %f, %f", main_camera.position.x, main_camera.position.y, main_camera.position.z);

	ui_draw_text_colored(200, 10, (const char*)buffer, 0xFFFFFFFF);

	snprintf(buffer, sizeof(buffer), "sun angle: %f", sun_angle);

	ui_draw_text_colored(500, 10, (const char*)buffer, 0xFFFFFFFF);

	ui_end_frame();
}

void game_shutdown(void)
{
	achievements_shutdown();
	world_free(&world);
}
