#ifdef _WIN32

#include "platform/platform.h"
#include "core/version.h"
#include "resources.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool running = false;
static HWND hwnd;

const uint8_t* g_asset_volumes[MAX_VOLUMES] = { 0 };
static HANDLE g_volume_files[MAX_VOLUMES]   = { 0 };
static HANDLE g_volume_maps[MAX_VOLUMES]    = { 0 };

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CLOSE:
		running = false;
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LPCWSTR convert_char_to_lpcwstr(const char* charArray)
{
	static WCHAR wideBuffer[32];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wideBuffer, 32);
	return wideBuffer;
}

void platform_init(const PlatformWindowDesc* desc)
{
	timeBeginPeriod(1);

	const char *className = "GameWindow";
	char title[128];
	snprintf(title, sizeof(title), "%s v%s", desc->title, gb_version);

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = window_proc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = className;

	RegisterClass(&wc);

	// LPCWSTR windowName = convert_char_to_lpcwstr(desc->title);

	hwnd = CreateWindow(
		className, title,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		desc->width, desc->height,
		NULL, NULL, wc.hInstance, NULL
	);

	if (!hwnd)
	{
		MessageBox(NULL, "Failed to create window.", "Error", MB_OK | MB_ICONERROR);
		return;
	}

	HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
	if (hIcon)
	{
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	}

	running = true;
}

void platform_shutdown(void)
{
	running = false;
	timeEndPeriod(1);
	if (hwnd) DestroyWindow(hwnd);
}

void platform_poll_events(void)
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool platform_running(void)
{
	return running;
}

float platform_frame_timing(void)
{
	static LARGE_INTEGER freq, prev = { 0 }, now;
	static bool initialized = false;

	if (!initialized)
	{
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&prev);
		initialized = true;
	}

	QueryPerformanceCounter(&now);
	double elapsed = (double)(now.QuadPart - prev.QuadPart) / freq.QuadPart;
	double frame_target = 1.0 / TARGET_FPS;

	if (elapsed < frame_target)
	{
		DWORD sleepMs = (DWORD)((frame_target - elapsed) * 1000);
		Sleep(sleepMs);
		QueryPerformanceCounter(&now);
		elapsed = (double)(now.QuadPart - prev.QuadPart) / freq.QuadPart;
	}

	prev = now;
	return (float)elapsed;
}

void* platform_get_native_window(void)
{
	return (void*)hwnd;
}

void platform_set_window_title(const char* title)
{
    if (hwnd)
    {
        SetWindowTextA(hwnd, title);
    }
}

void platform_init_assets(int32_t total_volumes)
{
	for (int32_t i = 0; i < total_volumes && i < MAX_VOLUMES; i++)
	{
		char filename[512];
		char exe_path[MAX_PATH] = {0};
		char base_dir[MAX_PATH] = {0};
		char candidate_paths[4][512] = {0};
		int32_t candidate_count = 0;

		GetModuleFileNameA(NULL, exe_path, sizeof(exe_path));
		for (int32_t j = (int32_t)strlen(exe_path) - 1; j >= 0; j--)
		{
			if (exe_path[j] == '\\' || exe_path[j] == '/')
			{
				exe_path[j] = '\0';
				break;
			}
		}
		if (exe_path[0] != '\0')
		{
			_snprintf_s(base_dir, sizeof(base_dir), _TRUNCATE, "%s", exe_path);
			_snprintf_s(candidate_paths[candidate_count++], sizeof(candidate_paths[0]), _TRUNCATE, "%s\\data\\data_%03d.dat", base_dir, i);
			_snprintf_s(candidate_paths[candidate_count++], sizeof(candidate_paths[0]), _TRUNCATE, "%s\\data_%03d.dat", base_dir, i);
		}
		_snprintf_s(candidate_paths[candidate_count++], sizeof(candidate_paths[0]), _TRUNCATE, "data\\data_%03d.dat", i);
		_snprintf_s(candidate_paths[candidate_count++], sizeof(candidate_paths[0]), _TRUNCATE, "data_%03d.dat", i);

		for (int32_t c = 0; c < candidate_count; c++)
		{
			if (candidate_paths[c][0] == '\0')
				continue;

			g_volume_files[i] = CreateFileA(
				candidate_paths[c],
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
				NULL
			);

			if (g_volume_files[i] != INVALID_HANDLE_VALUE)
			{
				_snprintf_s(filename, sizeof(filename), _TRUNCATE, "%s", candidate_paths[c]);
				break;
			}
		}

		if (g_volume_files[i] == INVALID_HANDLE_VALUE)
		{
			fprintf(stderr, "[platform] missing asset volume: %s\n", filename);
			continue;
		}

		g_volume_maps[i] = CreateFileMappingA(
			g_volume_files[i],
			NULL,
			PAGE_READONLY,
			0, 0,
			NULL
		);

		if (g_volume_maps[i])
		{
			g_asset_volumes[i] = (const uint8_t*)MapViewOfFile(g_volume_maps[i], FILE_MAP_READ, 0, 0, 0);
			if (!g_asset_volumes[i])
			{
				fprintf(stderr, "[platform] failed to map asset volume: %s\n", filename);
			}
		}
	}
}

void platform_shutdown_assets(int32_t total_volumes)
{
	for (int32_t i = 0; i < total_volumes; i++)
	{
		if (g_asset_volumes[i])
		{
			UnmapViewOfFile(g_asset_volumes[i]);
			g_asset_volumes[i] = NULL;
		}
		
		if (g_volume_maps[i])
		{
			CloseHandle(g_volume_maps[i]);
			g_volume_maps[i] = NULL;
		}

		if (g_volume_files[i] != INVALID_HANDLE_VALUE && g_volume_files[i] != NULL)
		{
			CloseHandle(g_volume_files[i]);
			g_volume_files[i] = INVALID_HANDLE_VALUE;
		}
	}
}

#endif // _WIN32