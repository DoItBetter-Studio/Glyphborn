#ifdef _WIN32

#include "platform.h"
#include "version.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <stdio.h>

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

void platform_init_assets(int total_volumes)
{
	for (int i = 0; i < total_volumes && i < MAX_VOLUMES; i++)
	{
		char filename[64];
		snprintf(filename, sizeof(filename), "data/data_%03d.dat", i);

		g_volume_files[i] = CreateFileA(
			filename, 
			GENERIC_READ, 
			FILE_SHARE_READ, 
			NULL, 
			OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, 
			NULL
		);

		if (g_volume_files[i] == INVALID_HANDLE_VALUE)
			continue;

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
		}
	}
}

void platform_shutdown_assets(int total_volumes)
{
	for (int i = 0; i < total_volumes; i++)
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