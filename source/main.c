#include <stdio.h>

#include "platform.h"
#include "render.h"
#include "game.h"
#include "audio.h"
#include "achievements.h"

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
	render_init(platform_get_native_window());
	audio_init();
	game_init();

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