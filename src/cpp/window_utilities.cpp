#include "window_utilities.hpp"

HBITMAP hBitmap;
BITMAP bitmapInfo;
float scaleX, scaleY;
RECT clientRect;
bool isFullscreen = false;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

HBITMAP LoadBitmapFromFile(const std::wstring& filename)
{
	return static_cast<HBITMAP>(LoadImage(NULL, filename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
}

void UpdateBitmap(HWND hWnd)
{
	// update the bitmap (that may or may not have actually changed)
	HBITMAP hNewBitmap = LoadBitmapFromFile(L"output.bmp");
	if (hNewBitmap != NULL)
	{
		// delete the old bitmap
		if (hBitmap != NULL)
			DeleteObject(hBitmap);
		hBitmap = hNewBitmap;

		// get the bitmap info and target window size
		GetObject(hBitmap, sizeof(BITMAP), &bitmapInfo);
		GetClientRect(hWnd, &clientRect);

		// calculate scaling factors (to fit the bitmap to the window)
		scaleX = static_cast<float>(bitmapInfo.bmWidth)  / static_cast<float>(std::max(1L, clientRect.right - clientRect.left));
		scaleY = static_cast<float>(bitmapInfo.bmHeight) / static_cast<float>(std::max(1L, clientRect.bottom - clientRect.top));

		// trigger a repaint
		InvalidateRect(hWnd, NULL, TRUE);
	}
}

void ToggleFullscreen(HWND hWnd)
{
	// toggle fullscreen mode
	isFullscreen = !isFullscreen;

	if (isFullscreen)
	 {
		GetWindowPlacement(hWnd, &wpPrev);
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~WS_OVERLAPPEDWINDOW);
		ShowWindow(hWnd, SW_MAXIMIZE);
	}
	else
	{
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(hWnd, &wpPrev);
		ShowWindow(hWnd, SW_NORMAL);

		RECT rc = { 0, 0, 960, 540 };
		AdjustWindowRect(&rc, GetWindowLong(hWnd, GWL_STYLE), FALSE);
		SetWindowPos(hWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
	}
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			// create the bitmap and set a timer to update it every second
			UpdateBitmap(hWnd);
			SetTimer(hWnd, 1, 1000, NULL);
			return 0;

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			HDC hdcMem = CreateCompatibleDC(hdc);
			SelectObject(hdcMem, hBitmap);
			SetStretchBltMode(hdc, HALFTONE);

			// draw the bitmap with scaling factors applied
			StretchBlt(hdc, 0, 0, 
					   static_cast<int>(bitmapInfo.bmWidth / scaleX), 
					   static_cast<int>(bitmapInfo.bmHeight / scaleY), 
					   hdcMem, 0, 0, bitmapInfo.bmWidth, bitmapInfo.bmHeight, SRCCOPY);

			DeleteDC(hdcMem);
			EndPaint(hWnd, &ps);
			return 0;
		}

		case WM_KEYDOWN: {
			// toggle fullscreen on F11 key
			if (wParam == VK_F11)
				ToggleFullscreen(hWnd);
			return 0;
		}

		case WM_TIMER:
			UpdateBitmap(hWnd);
			return 0;

		case WM_DESTROY:
			if (hBitmap != NULL)
				DeleteObject(hBitmap);
			KillTimer(hWnd, 1);
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Win32Window()
{
	// create the window class
	HINSTANCE hInstance = GetModuleHandle(NULL);
	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"OpenCLRayTracer";
	RegisterClass(&wc);

	// create the window
	HWND hWnd = CreateWindowExW(
		0,                            // extended window style
		L"OpenCLRayTracer",           // class name
		L"OpenCL Ray Tracer",          // window name
		WS_OVERLAPPEDWINDOW,          // window style
		CW_USEDEFAULT, CW_USEDEFAULT, // x, y
		960, 540,                     // width, height
		NULL,                         // parent window
		NULL,                         // menu
		hInstance,                    // instance handle
		NULL                          // additional application data
	);

	ShowWindow(hWnd, SW_SHOWDEFAULT);

	// message loop
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}