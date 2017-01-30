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

#include "Main.h"



// Global data:
static const wchar_t *Title = L"Cao";

static const UINT IconMessage = WM_APP + 9;
static NOTIFYICONDATA IconData = { 0 };
static HMENU IconMenu;
static const UINT __IconMenu_MessageIdStart = 1337;
static const UINT IconMenu_Run = __IconMenu_MessageIdStart + 0;
static const UINT IconMenu_Exit = __IconMenu_MessageIdStart + 1;

static HWND MyWindow = NULL;

static HHOOK KeyboardHook;



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
		SetConsoleTitle(Title);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
        SetConsoleCtrlHandler(ConsoleCtrlHandler, true);
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
	WindowClass.lpszClassName = Title;
	WindowClass.hIconSm       = LoadIcon(WindowClass.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&WindowClass))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			Title,
			NULL);

		return 1;
	}



	// Set up window:
	MyWindow = CreateWindow(
		WindowClass.lpszClassName,
		Title,
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
			Title,
			NULL);

		return 1;
	}

	// TODO: Only show window on click or keyboard shortcut.
	//ShowWindow(MyWindow, nCmdShow);
	UpdateWindow(MyWindow);



	// Set up cleanner upper for the notification icon no matter what happens. (Except stopping debugging still leaves garbage in the tray.)
	SetUnhandledExceptionFilter(UnexpectedExitHandler);


	

    // Set up tray icon menu:
    IconMenu = CreatePopupMenu();
    AppendMenu(IconMenu, MF_STRING, IconMenu_Run,  L"Run");
    AppendMenu(IconMenu, MF_SEPARATOR, 0,  NULL);
    AppendMenu(IconMenu, MF_STRING, IconMenu_Exit,  L"Exit");



	// Set up tray icon:
	IconData.cbSize           = sizeof(NOTIFYICONDATA);
	IconData.hWnd             = MyWindow;
	IconData.uID              = 0;
    // {DB649CB7-81B4-4638-A97D-25552A45D5C8}
	IconData.guidItem         = { 0xdb649cb7, 0x81b4, 0x4638,{ 0xa9, 0x7d, 0x25, 0x55, 0x2a, 0x45, 0xd5, 0xc8 } };
	IconData.hBalloonIcon     = NULL;
	IconData.uFlags           = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_GUID;
	IconData.uCallbackMessage = IconMessage;
	IconData.dwState          = 0;
	IconData.dwStateMask      = 0;
	IconData.uVersion         = NOTIFYICON_VERSION_4;
    IconData.dwInfoFlags      = 0;
    IconData.hIcon = LoadIcon(NULL, IDI_ASTERISK);
    StringCchCopy(IconData.szTip,       ARRAYSIZE(IconData.szTip),       Title);
    StringCchCopy(IconData.szInfoTitle, ARRAYSIZE(IconData.szInfoTitle), Title);
    StringCchCopy(IconData.szInfo,      ARRAYSIZE(IconData.szInfo),      Title);

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
                L"Call to Shell_NotifyIcon failed!",
                Title,
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

void
CleanupStuff()
{
    UnhookWindowsHookEx(KeyboardHook);
    Shell_NotifyIcon(NIM_DELETE, &IconData);
}

void
Run()
{
    printf("Running.\n");
    
	// Get clipboard data:
	// Try to open the clipboard.
	if (!OpenClipboard(MyWindow))
	{
		// On failure, give up;
		return;
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
		switch (currentFormat)
		{
			case CF_UNICODETEXT:
            {
				isText = true;
				break;
            }

			case CF_DIB:
            {
				isBitmap = true;
				break;
            }

			default:
            {
				currentFormat = EnumClipboardFormats(currentFormat);
				break;
            }
		}

	} while (currentFormat != firstFormat && !isText && !isBitmap);
    


	if (isText)
	{
		HGLOBAL textDataHandle = GetClipboardData(CF_UNICODETEXT);
		if (textDataHandle == NULL)
		{
			goto exit;
		}

		wchar_t *text = static_cast<wchar_t*>(GlobalLock(textDataHandle));
		if (text == NULL)
		{
			GlobalUnlock(text);
			goto exit;
		}

		printf("Text:\n%ls\n\n\n", text);



        // Start of child process creation:
        
        HANDLE Child_In_Read   = NULL;
        HANDLE Child_In_Write  = NULL;
        HANDLE Child_Out_Read  = NULL;
        HANDLE Child_Out_Write = NULL;


        SECURITY_ATTRIBUTES secAttr;
        secAttr.nLength              = sizeof(secAttr);
        secAttr.bInheritHandle       = true;
        secAttr.lpSecurityDescriptor = NULL;

        bool callResult = false;
        callResult =
            CreatePipe(
                &Child_Out_Read,
                &Child_Out_Write,
                &secAttr,
                0);

        if (!callResult)
        {
            // @logging log error.
            printf("Could not create pipe!\n");
            goto exit;
        }

        callResult = SetHandleInformation(Child_Out_Read, HANDLE_FLAG_INHERIT, 0);
        if (!callResult)
        {
            auto error = GetLastError();

            // @logging log error.
            printf("Could not set pipe information!\n");
            goto exit;
        }
        
        
        callResult =
            CreatePipe(
                &Child_In_Read,
                &Child_In_Write,
                &secAttr,
                0);
        if (!callResult)
        {
            // @logging log error.
            printf("Could not create pipe!\n");
            goto exit;
        }

        callResult = SetHandleInformation(Child_In_Write, HANDLE_FLAG_INHERIT, 0);
        if (!callResult)
        {
            // @logging log error.
            printf("Could not set pipe information!\n");
            goto exit;
        }
        
        wchar_t commandLine[4096] = L"ping localhost";
  
        STARTUPINFO startupInfo = { 0 };
        startupInfo.cb         = sizeof(startupInfo);
        startupInfo.hStdError  = Child_Out_Write;
        startupInfo.hStdOutput = Child_Out_Write;
        startupInfo.hStdInput  = Child_In_Read;
        startupInfo.dwFlags    = STARTF_USESTDHANDLES;
         
        PROCESS_INFORMATION procInfo = { 0 }; 
    
        bool success = false;
        success = CreateProcess(
            NULL, 
            commandLine,
            NULL,          
            NULL,          
            true,          
            0,             
            NULL,          
            NULL,          
            &startupInfo,  
            &procInfo); 

        if (!success)
        {
            printf("Could not start child process: %ls", commandLine);
            goto exit;
        }
        else
        {
            // Close handles to the child process and its primary thread.
            // Some applications might keep these handles to monitor the status
            // of the child process, for example. 

            CloseHandle(procInfo.hProcess);
            CloseHandle(procInfo.hThread);
        }


        /*
        HANDLE inputFile =
            CreateFile(
                L"inputFile", 
                GENERIC_READ, 
                0, 
                NULL, 
                OPEN_EXISTING, 
                FILE_ATTRIBUTE_READONLY, 
                NULL);
        */

        /*
        DWORD written;

        char buffer[5] = "test";

        bool wasWritten = false;
        wasWritten = WriteFile(Child_In_Write, buffer, 5, &written, NULL);
        */
        
            CHAR chBuf[4096]; 

        { 
            DWORD dwRead, dwWritten; 
            BOOL bSuccess = FALSE;
            HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

            for (;;) 
            { 
                bSuccess = ReadFile(Child_Out_Read, chBuf, 4096, &dwRead, NULL);
                if( ! bSuccess || dwRead == 0 ) break; 

                bSuccess = WriteFile(hParentStdOut, chBuf, 
                                    dwRead, &dwWritten, NULL);
                if (! bSuccess ) break; 
            } 
        }
 
        printf("MAde it out.");




		GlobalUnlock(textDataHandle);
	}
	else
	{
		// Error or no data.
        goto exit;
	}

exit:
	CloseClipboard();
}

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
                Run();
			}
		}
	}

	return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK
WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR greeting[] = _T("Hello, World!");

    switch (message)
    {
        case WM_PAINT:
        {
            /*
            hdc = BeginPaint(Window, &ps);

            TextOut(hdc, 5, 5, greeting, _tcslen(greeting));
            
            */
            EndPaint(Window, &ps);
            break;
        }

        case WM_DESTROY:
        {
            CleanupStuff();
            PostQuitMessage(0);
            break;
        }

        case IconMessage:
        {
            switch (lParam)
            {
                case WM_LBUTTONDBLCLK:
                {
                    //ShowWindow(hWnd, SW_RESTORE);
                    printf("Left clicked!\n");
                    break;
                }

                case WM_RBUTTONDOWN:
                {
                    printf("Right clicked!\n");

                    POINT CursorPoint;
                    GetCursorPos(&CursorPoint);

                    SetForegroundWindow(Window);

                    UINT clicked =
                        TrackPopupMenu(
                            IconMenu,
                            TPM_RETURNCMD | TPM_NONOTIFY,
                            CursorPoint.x,
                            CursorPoint.y,
                            0,
                            Window,
                            NULL);
                    switch (clicked)
                    {
                        case IconMenu_Exit:
                        {
                            printf("Exit clicked!\n");
                            DestroyWindow(Window);
                            break;
                        }
                        case IconMenu_Run:
                        {
                            printf("Run clicked!\n");
                            Run();
                            break;
                        }
                        default:
                        {
                            printf("No idea: %d", clicked);
                            break;
                        }
                    }

                    break;
                }
            }

            break;
        }

        default:
        {
            return DefWindowProc(Window, message, wParam, lParam);
            break;
        }
    }

	return 0;
}

LONG WINAPI
UnexpectedExitHandler(PEXCEPTION_POINTERS pExceptionInfo)
{
    CleanupStuff();

    return EXCEPTION_CONTINUE_SEARCH;
}

BOOL WINAPI
ConsoleCtrlHandler(
    DWORD controlType
)
{
    switch (controlType)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
        case CTRL_CLOSE_EVENT:
        {
            CleanupStuff();
            break;
        }
    }

    return false; 
}