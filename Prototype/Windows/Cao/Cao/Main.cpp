// Compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c  
#pragma comment( lib, "user32.lib" )

#include <windows.h>  
#include <stdlib.h>  
#include <string.h>  
#include <tchar.h>
#include <strsafe.h>
#include <string>

// Globals (move these later):
static const UINT MY_ICON_MESSAGE = WM_APP + 9;
static NOTIFYICONDATA nid = { 0 };
static HWND myWindow = NULL;

static HHOOK hKeyboardHook;

LRESULT CALLBACK
KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
	CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);

	DWORD SHIFT_key = 0;
	DWORD CTRL_key = 0;
	DWORD ALT_key = 0;

	if ((nCode == HC_ACTION) && ((wParam == WM_SYSKEYDOWN) || (wParam == WM_KEYDOWN)))
	{
		KBDLLHOOKSTRUCT HookedKey = *((KBDLLHOOKSTRUCT*)lParam);
		
		int key = HookedKey.vkCode;

		SHIFT_key = GetAsyncKeyState(VK_SHIFT);
		CTRL_key = GetAsyncKeyState(VK_CONTROL);
		ALT_key = GetAsyncKeyState(VK_MENU);

		if (key >= 'A' && key <= 'Z')
		{
			if (GetAsyncKeyState(VK_SHIFT) >= 0)
			{
				key += 32;
			}

			if (CTRL_key != 0 && key == 'y')
			{
				MessageBox(NULL, _T("CTRL-y was pressed\nLaunch your app here"), _T("H O T K E Y"), MB_OK);
			}
			else if (CTRL_key != 0 && ALT_key != 0 && key == 'c')
			{
				// Get clipboard data:
				if (!IsClipboardFormatAvailable(CF_TEXT))
				{
					return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				}

				if (!OpenClipboard(myWindow))
				{
					return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				}

				HANDLE hData = GetClipboardData(CF_TEXT);
				if (hData == NULL)
				{
					return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				}

				// Get (ASCII?) text from the clipboard and display it.
				char* text = static_cast<char*>(GlobalLock(hData));
				if (text == NULL)
				{
					GlobalUnlock(hData);
					CloseClipboard();
					return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
				}

				int textLen = lstrlenA(text);
				BSTR conText = SysAllocStringLen(NULL, textLen);
				::MultiByteToWideChar(CP_ACP, 0, text, textLen, conText, textLen);
				OutputDebugString(conText);
				SysFreeString(conText);

				GlobalUnlock(hData);
				CloseClipboard();

				//MessageBox(NULL, _T("CTRL-c was pressed\nData:"), _T("Cao"), MB_OK);
			}
			/*
			else if (CTRL_key != 0 && key == 'q')
			{
				MessageBox(NULL, _T("Shutting down"), _T("H O T K E Y"), MB_OK);
			}
			*/
		}

	}

	return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
}

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
		UnhookWindowsHookEx(hKeyboardHook);
		Shell_NotifyIcon(NIM_DELETE, &nid);
		PostQuitMessage(0);
		break;

	case MY_ICON_MESSAGE:
		OutputDebugString(_T("My icon sent a message.\n"));

		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			ShowWindow(hWnd, SW_RESTORE);
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			//ShowContextMenu(hWnd);
			break;
		}
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
	myWindow = CreateWindow(
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

	if (!myWindow)
	{
		MessageBox(
			NULL,
			_T("Call to CreateWindow failed!"),
			_T("Cao"),
			NULL);

		return 1;
	}

	ShowWindow(myWindow, nCmdShow);
	UpdateWindow(myWindow);



	// Set up tray icon:
	// {DB649CB7-81B4-4638-A97D-25552A45D5C8}
	static const GUID myIconGUID =
	{ 0xdb649cb7, 0x81b4, 0x4638, { 0xa9, 0x7d, 0x25, 0x55, 0x2a, 0x45, 0xd5, 0xc8 } };

	//NOTIFYICONDATA nid = { 0 };
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = myWindow;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_GUID;
	nid.uCallbackMessage = MY_ICON_MESSAGE;
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
			myWindow,
			_T("Call to Shell_NotifyIcon failed!"),
			_T("Cao"),
			NULL);

		return 1;
	}



	// Set up global hook:
	hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardEvent, hInstance, NULL);

	

	// Main message loop:  
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
