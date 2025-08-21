#include <stdio.h>

#include "platform.h"
#include "render.h"
#include "audio.h"
#include "mapper.h"

//#ifdef _WIN32
//#include <Windows.h>
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
//#else
//int main()
//#endif
int main()
{
	PlatformWindowDesc window = { 0 };
	window.width = 1280;
	window.height = 720;
	window.title = "Mapper";

	platform_init(&window);
	render_init(platform_get_native_window());
	//audio_init();
	mapper_init();

	while (platform_running())
	{
		platform_poll_events();

		render_clear(framebuffer, 0xFF333333);
		render_clear(framebuffer_ui, 0xFF333333);

		mapper_update(platform_frame_timing());
		//audio_update();

		mapper_render();

		mapper_render_ui();

		render_blend_ui_under_mapview();
		render_present();
	}

	mapper_shutdown();
	//audio_shutdown();
	render_shutdown();
	platform_shutdown();
	return 0;
}