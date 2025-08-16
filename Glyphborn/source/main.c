#include <stdio.h>

#include "platform.h"
#include "render.h"
#include "game.h"
#include "audio.h"

#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main()
#endif
{
	PlatformWindowDesc window = { 0 };
	window.width = 1280;
	window.height = 720;
	window.title = "Glyphborn";

	platform_init(&window);
	render_init(platform_get_native_window());
	game_init();
	audio_init();

	while (platform_running())
	{
		platform_poll_events();

		render_clear(framebuffer, 0xFF000000);
		render_clear(framebuffer_game, 0xFFAAAAAA);
		render_clear(framebuffer_ui, 0x00000000);

		game_update(platform_frame_timing());
		audio_update();

		game_render();
		game_render_ui();

		render_blend_ui_over_game();
		render_present();
	}

	audio_shutdown();
	game_shutdown();
	render_shutdown();
	platform_shutdown();
	return 0;
}