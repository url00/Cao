// Compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c  

#include <windows.h>  
#include <stdlib.h>  
#include <string.h>  
#include <tchar.h>
#include <strsafe.h>

LRESULT CALLBACK
WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR greeting[] = _T("Hello, World!");

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		TextOut(hdc, 5, 5, greeting, _tcslen(greeting));

		EndPaint(hWnd, &ps);

		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

int CALLBACK
WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	// Set up applicaion:
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _T("cao");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Win32 Guided Tour"),
			NULL);

		return 1;
	}



	// Set up window:
	HWND hWnd = CreateWindow(
		wcex.lpszClassName,
		_T("Cao"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 100,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(
			NULL,
			_T("Call to CreateWindow failed!"),
			_T("Cao"),
			NULL);

		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);



	// Set up tray icon:
	// {DB649CB7-81B4-4638-A97D-25552A45D5C8}
	static const GUID myIconGUID =
	{ 0xdb649cb7, 0x81b4, 0x4638, { 0xa9, 0x7d, 0x25, 0x55, 0x2a, 0x45, 0xd5, 0xc8 } };

	NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_GUID;
	nid.uCallbackMessage = 9;
	nid.hIcon = LoadIcon(NULL, IDI_ASTERISK);
	StringCchCopy(nid.szTip, ARRAYSIZE(nid.szTip), _T("A icon?"));
	nid.dwState = 0;
	nid.dwStateMask = 0;
	StringCchCopy(nid.szInfo, ARRAYSIZE(nid.szInfo), _T("A icon?"));
	nid.uVersion = NOTIFYICON_VERSION_4;
	StringCchCopy(nid.szInfoTitle, ARRAYSIZE(nid.szInfoTitle), _T("A icon?"));
	nid.dwInfoFlags = 0;
	nid.guidItem = myIconGUID;
	nid.hBalloonIcon = NULL;

	// Add the icon.
	BOOL wasIconCreated = Shell_NotifyIcon(NIM_ADD, &nid);
	if (!wasIconCreated)
	{
		MessageBox(
			hWnd,
			_T("Call to Shell_NotifyIcon failed!"),
			_T("Cao"),
			NULL);

		return 1;
	}


	
	// Main message loop:  
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
