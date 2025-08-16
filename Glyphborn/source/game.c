#include "game.h"
#include "input.h"
#include "render.h"
#include "camera.h"
#include "test_cube.h"
#include "windowskin_0.h"
#include "audio.h"
#include "audio/player_jump.h"
#include "ui.h"

Camera main_camera = { 0 };
Mat4 view;
Mat4 projection;

UISystem ui;

static void draw_test_pattern_ui(void)
{
	for (int y = 200; y < FB_HEIGHT-16; ++y)
	{
		for (int x = 16; x < FB_WIDTH-16; ++x)
		{
			bool is_light = ((x / 16) + (y / 16)) % 2 == 0;
			uint32_t color = is_light ? 0x88FFFFFF : 0x88000000; // semi-transparent white/black
			framebuffer_ui[y * FB_WIDTH + x] = color;
		}
	}
}

static void on_click_jump(void* data)
{
	audio_play_sound(player_jump, PLAYER_JUMP_LEN);
}

void game_init(void)
{
	input_init();
	ui_init(&ui);

	main_camera.position = (Vec3){ 0.0f, 0.0f, -5.0f };
	main_camera.target = (Vec3){ 0.0f, 0.0f, 0.0f };
	main_camera.up = (Vec3){ 0.0f, 1.0f, 0.0f };

	view = camera_get_view_matrix(&main_camera);
	projection = mat4_perspective(3.14159f / 4.0f, (float)FB_WIDTH / (float)FB_HEIGHT, 0.1f, 100.0f);

	ui_add_element(&ui, ui_create_button(50, 50, 120, 32, "Click Me", 0xFF333333, 0xFF000000, on_click_jump));
	ui_add_element(&ui, ui_create_button(50, 100, 120, 32, "Button 2", 0xFF333333, 0xFF000000, on_click_jump));

	ui_add_element(&ui, ui_create_label(50, 200, "Hello, UI!", 0xFF0000FF)); // Blue text
}

static bool activate = false;

void game_update(float delta_time)
{
	input_update();

	int nav_dx = 0, nav_dy = 0;

	if (input_button_down(BUTTON_START))
	{
		ui.input_locked = !ui.input_locked; // Toggle input lock
	}

	if (ui.input_locked)
	{
		if (input_button_down(BUTTON_UP)) nav_dy += 1;
		if (input_button_down(BUTTON_DOWN)) nav_dy += -1;
		if (input_button_down(BUTTON_LEFT)) nav_dx += -1;
		if (input_button_down(BUTTON_RIGHT)) nav_dx += 1;
		if (input_button_down(BUTTON_A)) activate = true;
	}

	// TODO: Implement mouse input
	ui_update(&ui, delta_time, 0, 0, false, nav_dx, nav_dy, activate);
	activate = false; // Reset activation state after processing

	if (input_button_down(BUTTON_B))
	{
		audio_play_sound(player_jump, PLAYER_JUMP_LEN);
	}

	if (!ui.input_locked)
	{
		if (input_get_button(BUTTON_UP))
			main_camera.position.z += 1.0f * delta_time;
		if (input_get_button(BUTTON_DOWN))
			main_camera.position.z -= 1.0f * delta_time;
		if (input_get_button(BUTTON_LEFT))
			main_camera.position.x -= 1.0f * delta_time;
		if (input_get_button(BUTTON_RIGHT))
			main_camera.position.x += 1.0f * delta_time;
	}

	view = camera_get_view_matrix(&main_camera);
}

void game_render(void)
{
	Mat4 model = mat4_identity();

	RenderMesh cube_mesh = {
		.vertices = cube_vertices,
		.vertex_count = sizeof(cube_vertices) / sizeof(Vec3),
		.edges = cube_edges,
		.edge_count = sizeof(cube_edges) / (2 * sizeof(int))
	};

	render_draw_wireframe(&cube_mesh, model, view, projection, 0xFF000000);
}

void game_render_ui(void)
{
	ui_render(&ui);
}

void game_shutdown(void)
{
	ui_clear(&ui);
}
