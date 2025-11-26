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

void game_init(void)
{
	input_init();
	achievements_init();

	ui_set_skin(SKIN_GLYPHBORN);

	main_camera.position = (Vec3){ 0.0f, 0.0f, -5.0f };
	main_camera.target = (Vec3){ 0.0f, 0.0f, 0.0f };
	main_camera.up = (Vec3){ 0.0f, 1.0f, 0.0f };

	view = camera_get_view_matrix(&main_camera);
	projection = mat4_perspective(3.14159f / 4.0f, (float)FB_WIDTH / (float)FB_HEIGHT, 0.1f, 100.0f);
}

static bool activate = false;
int nav_dx = 0, nav_dy = 0;
static bool showUI = false;

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
		audio_play_sound(player_jump, PLAYER_JUMP_LEN);
	}

	if (input_get_button(BUTTON_UP)) main_camera.position.z += 1.0f * delta_time;
	if (input_get_button(BUTTON_DOWN)) main_camera.position.z -= 1.0f * delta_time;
	if (input_get_button(BUTTON_LEFT)) main_camera.position.x -= 1.0f * delta_time;
	if (input_get_button(BUTTON_RIGHT)) main_camera.position.x += 1.0f * delta_time;

	view = camera_get_view_matrix(&main_camera);
}

void game_render(void)
{ }

void game_render_ui(void)
{
	ui_begin_frame(0, 0, false, activate);

	if (nav_dx || nav_dy)
	{
		g_ui.nav_mode = true;
		g_ui.focused_id += nav_dy;
		if (g_ui.focused_id < 1) g_ui.focused_id = 1;
	}

	if (ui_button(16, 16, 100, 30, "Jump", 0xFFFFFFFF))
	{
		audio_play_sound(player_jump, PLAYER_JUMP_LEN);
	}

	if (showUI)
	{
		draw_test_pattern_ui();
	}

	ui_end_frame();
}

void game_shutdown(void)
{
	achievements_shutdown();
}
