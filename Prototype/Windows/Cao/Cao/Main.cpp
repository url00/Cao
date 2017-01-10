// Compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c  
#pragma comment( lib, "user32.lib" )

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

// Bitmap structs:
typedef struct
{
	std::uint32_t biSize;
	std::int32_t  biWidth;
	std::int32_t  biHeight;
	std::uint16_t  biPlanes;
	std::uint16_t  biBitCount;
	std::uint32_t biCompression;
	std::uint32_t biSizeImage;
	std::int32_t  biXPelsPerMeter;
	std::int32_t  biYPelsPerMeter;
	std::uint32_t biClrUsed;
	std::uint32_t biClrImportant;
} DIB;

typedef struct
{
	std::uint16_t type;
	std::uint32_t bfSize;
	std::uint32_t reserved;
	std::uint32_t offset;
} HEADER;

typedef struct
{
	HEADER header;
	DIB dib;
} BMP;


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
			else if (CTRL_key != 0 && key == 'e')
			{
				// Get clipboard data:
				// Try to open the clipboard.
				if (!OpenClipboard(myWindow))
				{
					// On failure, give up and let the next hook in the chain run.
					return CallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
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

					HGLOBAL hBitmapData = GetClipboardData(CF_DIB);
					if (hBitmapData == NULL)
					{
						goto cleanup;
					}

					void* dib = GlobalLock(hBitmapData);
					if (dib == NULL)
					{
						GlobalUnlock(dib);
						goto cleanup;
					}

					DIB *info = reinterpret_cast<DIB*>(dib);
					BMP bmp = { 0 };
					bmp.header.type = 0x4D42;
					bmp.header.offset = 54;
					bmp.header.bfSize = info->biSizeImage + bmp.header.offset;
					bmp.dib = *info;

					
					std::cout << "Type: " << std::hex << bmp.header.type << std::dec << "\n";
					std::cout << "bfSize: " << bmp.header.bfSize << "\n";
					std::cout << "Reserved: " << bmp.header.reserved << "\n";
					std::cout << "Offset: " << bmp.header.offset << "\n";
					std::cout << "biSize: " << bmp.dib.biSize << "\n";
					std::cout << "Width: " << bmp.dib.biWidth << "\n";
					std::cout << "Height: " << bmp.dib.biHeight << "\n";
					std::cout << "Planes: " << bmp.dib.biPlanes << "\n";
					std::cout << "Bits: " << bmp.dib.biBitCount << "\n";
					std::cout << "Compression: " << bmp.dib.biCompression << "\n";
					std::cout << "Size: " << bmp.dib.biSizeImage << "\n";
					std::cout << "X-res: " << bmp.dib.biXPelsPerMeter << "\n";
					std::cout << "Y-res: " << bmp.dib.biYPelsPerMeter << "\n";
					std::cout << "ClrUsed: " << bmp.dib.biClrUsed << "\n";
					std::cout << "ClrImportant: " << bmp.dib.biClrImportant << "\n";
					

					std::ofstream file("C:/Users/Nathan/Desktop/Test.bmp", std::ios::out | std::ios::binary);
					if (file)
					{
						file.write(reinterpret_cast<char*>(&bmp.header.type), sizeof(bmp.header.type));
						file.write(reinterpret_cast<char*>(&bmp.header.bfSize), sizeof(bmp.header.bfSize));
						file.write(reinterpret_cast<char*>(&bmp.header.reserved), sizeof(bmp.header.reserved));
						file.write(reinterpret_cast<char*>(&bmp.header.offset), sizeof(bmp.header.offset));
						file.write(reinterpret_cast<char*>(&bmp.dib.biSize), sizeof(bmp.dib.biSize));
						file.write(reinterpret_cast<char*>(&bmp.dib.biWidth), sizeof(bmp.dib.biWidth));
						file.write(reinterpret_cast<char*>(&bmp.dib.biHeight), sizeof(bmp.dib.biHeight));
						file.write(reinterpret_cast<char*>(&bmp.dib.biPlanes), sizeof(bmp.dib.biPlanes));
						file.write(reinterpret_cast<char*>(&bmp.dib.biBitCount), sizeof(bmp.dib.biBitCount));
						file.write(reinterpret_cast<char*>(&bmp.dib.biCompression), sizeof(bmp.dib.biCompression));
						file.write(reinterpret_cast<char*>(&bmp.dib.biSizeImage), sizeof(bmp.dib.biSizeImage));
						file.write(reinterpret_cast<char*>(&bmp.dib.biXPelsPerMeter), sizeof(bmp.dib.biXPelsPerMeter));
						file.write(reinterpret_cast<char*>(&bmp.dib.biYPelsPerMeter), sizeof(bmp.dib.biYPelsPerMeter));
						file.write(reinterpret_cast<char*>(&bmp.dib.biClrUsed), sizeof(bmp.dib.biClrUsed));
						file.write(reinterpret_cast<char*>(&bmp.dib.biClrImportant), sizeof(bmp.dib.biClrImportant));
						file.write(reinterpret_cast<char*>(info + 1), bmp.dib.biSizeImage);
					}

					GlobalUnlock(dib);
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

int CALLBACK
WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	// Set up and display console for debugging:
	if (AllocConsole()) {
		FILE* pCout;
		freopen_s(&pCout, "CONOUT$", "w", stdout);
		SetConsoleTitle(L"Debug Console");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}

	OutBuffer buffer;
	std::streambuf *sb = std::cout.rdbuf(&buffer);
	std::cout.rdbuf(sb);



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
