// Compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c  
#pragma comment(lib, "user32.lib")

#include <windows.h>  
#include <stdlib.h>  
#include <string.h>  
#include <tchar.h>
#include <strsafe.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>

static const wchar_t *TITLE = L"Cao";

static const UINT MY_ICON_MESSAGE = WM_APP + 9;
static NOTIFYICONDATA IconData = { 0 };

static HWND MyWindow = NULL;

static HHOOK KeyboardHook;

LRESULT CALLBACK
KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam)
{
	CallNextHookEx(KeyboardHook, nCode, wParam, lParam);

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
			else if (CTRL_key != 0 && key == 'e')
			{
				// Get clipboard data:
				// Try to open the clipboard.
				if (!OpenClipboard(MyWindow))
				{
					// On failure, give up and let the next hook in the chain run.
					return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
				}



				// Find out which type of data we are dealing with.
				// Right now just handling text and bitmap.
				// Other possible options:
				//   CF_HDROP: Selected file in explorer.
				//   CF_WAVE: Wave file.
				UINT firstFormat = EnumClipboardFormats(0);
				UINT currentFormat = firstFormat;
				bool isText = false;
				bool isBitmap = false;
				do
				{
					std::cout << currentFormat << std::endl;
					switch (currentFormat)
					{
					case CF_UNICODETEXT:
						isText = true;
						break;

					case CF_DIB:
						isBitmap = true;
						break;

					default:
						currentFormat = EnumClipboardFormats(currentFormat);
						break;
					}

				} while (currentFormat != firstFormat && !isText && !isBitmap);



				if (isText)
				{
					HGLOBAL hTextData = GetClipboardData(CF_UNICODETEXT);
					if (hTextData == NULL)
					{
						goto cleanup;
					}

					wchar_t* text = static_cast<wchar_t*>(GlobalLock(hTextData));
					if (text == NULL)
					{
						GlobalUnlock(text);
						goto cleanup;
					}

					OutputDebugString(text);

					GlobalUnlock(text);
				}
				else if (isBitmap)
				{
					// TODO: Consider adding bitmap support.
				}
				else
				{
					// Error or no data.
				}

			cleanup:
				CloseClipboard();
			}
		}
	}

	return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
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
		UnhookWindowsHookEx(KeyboardHook);
		Shell_NotifyIcon(NIM_DELETE, &IconData);
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

// For debug console.
class OutBuffer : public std::streambuf
{
public:
	OutBuffer()
	{
		setp(0, 0);
	}

	virtual int_type
	overflow(int_type c = traits_type::eof())
	{
		return fputc(c, stdout) == EOF ? traits_type::eof() : c;
	}
};

LONG WINAPI
CleanupPersistentStuff(PEXCEPTION_POINTERS pExceptionInfo)
{
	UnhookWindowsHookEx(KeyboardHook);
	Shell_NotifyIcon(NIM_DELETE, &IconData);

    return EXCEPTION_CONTINUE_SEARCH;
}

int CALLBACK
WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	char      *cmdLine,
	int       cmdShow
)
{
	// Set up and display console for debugging:
	if (AllocConsole()) {
		FILE *cout;
		freopen_s(&cout, "CONOUT$", "w", stdout);
		SetConsoleTitle(TITLE);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}

	OutBuffer Buffer;
	std::streambuf *sb = std::cout.rdbuf(&Buffer);
	std::cout.rdbuf(sb);



	// Set up applicaion:
	WNDCLASSEX WindowClass    = { 0 };
	WindowClass.cbSize	       = sizeof(WNDCLASSEX);
	WindowClass.style         = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc   = WndProc;
	WindowClass.cbClsExtra    = 0;
	WindowClass.cbWndExtra    = 0;
	WindowClass.hInstance     = Instance;
	WindowClass.hIcon         = LoadIcon(Instance, MAKEINTRESOURCE(IDI_APPLICATION));
	WindowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WindowClass.lpszMenuName  = NULL;
	WindowClass.lpszClassName = TITLE;
	WindowClass.hIconSm       = LoadIcon(WindowClass.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&WindowClass))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			TITLE,
			NULL);

		return 1;
	}



	// Set up window:
	MyWindow = CreateWindow(
		WindowClass.lpszClassName,
		TITLE,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		500, 100,
		NULL,
		NULL,
		Instance,
		NULL
	);

	if (!MyWindow)
	{
		MessageBox(
			NULL,
			_T("Call to CreateWindow failed!"),
			TITLE,
			NULL);

		return 1;
	}

	// TODO: Only show window on click or keyboard shortcut.
	//ShowWindow(MyWindow, nCmdShow);
	UpdateWindow(MyWindow);



	// Set up cleanner upper for the notification icon no matter what happens. (Except stopping debugging still leaves garbage in the tray.)
	SetUnhandledExceptionFilter(CleanupPersistentStuff);


	

	// Set up tray icon:
	// {DB649CB7-81B4-4638-A97D-25552A45D5C8}
	static const GUID IconGUID =
	{ 0xdb649cb7, 0x81b4, 0x4638, { 0xa9, 0x7d, 0x25, 0x55, 0x2a, 0x45, 0xd5, 0xc8 } };

	//NOTIFYICONDATA nid = { 0 };
	IconData.cbSize = sizeof(NOTIFYICONDATA);
	IconData.hWnd = MyWindow;
	IconData.uID = 0;
	IconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_GUID;
	IconData.uCallbackMessage = MY_ICON_MESSAGE;
	IconData.hIcon = LoadIcon(NULL, IDI_ASTERISK);
	StringCchCopy(IconData.szTip, ARRAYSIZE(IconData.szTip), TITLE);
	IconData.dwState = 0;
	IconData.dwStateMask = 0;
	StringCchCopy(IconData.szInfo, ARRAYSIZE(IconData.szInfo), TITLE);
	IconData.uVersion = NOTIFYICON_VERSION_4;
	StringCchCopy(IconData.szInfoTitle, ARRAYSIZE(IconData.szInfoTitle), _T("A icon?"));
	IconData.dwInfoFlags = 0;
	IconData.guidItem = IconGUID;
	IconData.hBalloonIcon = NULL;

	// Add the icon.
	BOOL wasIconCreated = Shell_NotifyIcon(NIM_ADD, &IconData);
	if (!wasIconCreated)
	{
        // Most likely there is an instance of the icon left from a previous run.
        // Try to delete the existing instance.
        Shell_NotifyIcon(NIM_DELETE, &IconData);

        // Try to create an instance for this run.
        wasIconCreated = Shell_NotifyIcon(NIM_ADD, &IconData);
        if (!wasIconCreated)
        {
            // @Logging Log unable to start, couldn't create shell.
            MessageBox(
                MyWindow,
                _T("Call to Shell_NotifyIcon failed!"),
                TITLE,
                NULL);

            return 1;
        }
	}



	// Set up global hook:
	KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardEvent, Instance, NULL);



	// Main message loop:  
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
