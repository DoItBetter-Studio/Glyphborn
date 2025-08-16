#include "game.h"
#include "input.h"
#include "render.h"
#include "camera.h"
#include "test_cube.h"
#include "windowskin_0.h"

Camera main_camera = { 0 };
Mat4 view;
Mat4 projection;

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

void game_init(void)
{
	input_init();

	main_camera.position = (Vec3){ 0.0f, 0.0f, -5.0f };
	main_camera.target = (Vec3){ 0.0f, 0.0f, 0.0f };
	main_camera.up = (Vec3){ 0.0f, 1.0f, 0.0f };

	view = camera_get_view_matrix(&main_camera);
	projection = mat4_perspective(3.14159f / 4.0f, (float)FB_WIDTH / (float)FB_HEIGHT, 0.1f, 100.0f);
}

static bool showUI = false;

void game_update(float delta_time)
{
	input_update();

	if (input_button_down(BUTTON_A))
		showUI = !showUI;

	if (input_get_button(BUTTON_UP))
		main_camera.position.z += 1.0f * delta_time;
	if (input_get_button(BUTTON_DOWN))
		main_camera.position.z -= 1.0f * delta_time;
	if (input_get_button(BUTTON_LEFT))
		main_camera.position.x -= 1.0f * delta_time;
	if (input_get_button(BUTTON_RIGHT))
		main_camera.position.x += 1.0f * delta_time;

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
	if (showUI)
	{
		render_draw_image(10, 210, WINDOWSKIN_0_WIDTH, WINDOWSKIN_0_HEIGHT, WINDOWSKIN_0_BITDEPTH, windowskin_0, windowskin_0_palette);
		render_draw_text_colored(20, 220, "Hello, Glyphborn!", 0xFFFF0000); // Red text
	}
	// Draw a simple UI element
	for (int y = 10; y < 20; ++y)
	{
		for (int x = 50; x < FB_WIDTH - 50; ++x)
		{
			framebuffer_ui[y * FB_WIDTH + x] = 0xFF00FF00; // Green bar at the top
		}
	}
}

void game_shutdown(void)
{
}
