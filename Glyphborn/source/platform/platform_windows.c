#ifdef _WIN32

#include "platform.h"
#include "version.h"
#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>
#include <stdio.h>

static bool running = false;
static HWND hwnd;

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
#endif // _WIN32