#include <stdio.h>

#include "platform/platform.h"
#include "render/render.h"
#include "input/input.h"
#include "game/game.h"
#include "audio/audio.h"
#include "achievements/achievements.h"

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	// 1. Manually create a new console window for this process
    if (AllocConsole()) 
    {
        FILE* fDummy;
        // 2. Redirect the C Runtime's stdout and stderr to the new console
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        
		setvbuf(stdout, NULL, _IONBF, 0); // Disable buffering

        // Optional: Set a title so you know which console is yours
        SetConsoleTitleA("Glyphborn Debug Console");
    }

	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nShowCmd;
#elif __YGGDRASIL__
#include <yggdrasil.h>
extern void ygg_init_api(void*);

int main(void* api)
{
	ygg_init_api(api);

#else
int main()
{
#endif
	PlatformWindowDesc window = { 0 };
	window.width = 640;
	window.height = 360;
	window.title = "Glyphborn";

	platform_init(&window);

	platform_init_assets(1);

	render_init(platform_get_native_window());
	audio_init();
	input_init(platform_get_native_window());
	game_init();

	float true_fps_accumulator = 0.0f;
	int32_t true_frame_counter = 0;
	char title_buffer[128];

	while (platform_running())
	{
		platform_poll_events();

		render_clear(framebuffer, 0xFF000000);
		render_clear(framebuffer_game, 0xFFAAAAAA);
		render_clear(framebuffer_ui, 0x00000000);

		float delta_time = platform_frame_timing();

		// 2. Accumulate metrics right here at the engine's heartbeat
		true_frame_counter++;
		true_fps_accumulator += delta_time;

		if (true_fps_accumulator >= 1.0f)
		{
			snprintf(title_buffer, sizeof(title_buffer), 
					"Glyphborn | FPS: %d (%.2f ms)", 
					true_frame_counter, (1000.0f / (float)true_frame_counter));
			
			platform_set_window_title(title_buffer);

			true_frame_counter = 0;
			true_fps_accumulator -= 1.0f;
		}

		game_update(delta_time);
		audio_update();

		game_render();
		game_render_ui();

		render_blend_ui_over_game();
		render_present();
	}

	audio_shutdown();
	game_shutdown();
	render_shutdown();
	platform_shutdown_assets(1);
	platform_shutdown();
	return 0;
}