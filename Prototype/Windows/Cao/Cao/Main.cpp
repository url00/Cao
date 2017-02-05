// Compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c  
#pragma comment(lib, "user32.lib")

#include <Windows.h>
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



// Global data.
static const wchar_t *TITLE = L"Cao";
static HWND MyWindow = NULL;
static HHOOK KeyboardHook;
static HANDLE My_Out = NULL;



static const UINT IconMessage = WM_APP + 9;
static NOTIFYICONDATA IconData = { 0 };
static HMENU IconMenu;
static const UINT __IconMenu_MessageIdStart = 1337;
static const UINT IconMenu_RunCancel  = __IconMenu_MessageIdStart + 0;
static const UINT IconMenu_Exit       = __IconMenu_MessageIdStart + 1;



static bool isChildRunning    = false;
static bool wasChildCancelled = false;

static HANDLE Child_In_Read   = NULL;
static HANDLE Child_In_Write  = NULL;
static HANDLE Child_Out_Read  = NULL;
static HANDLE Child_Out_Write = NULL;

static PROCESS_INFORMATION ChildProcInfo = { 0 };



static const wchar_t *configFilename = L"config.txt";
static ConfigFile LoadedConfigFile = { 0 };
static int currentConfigIndex = 1;



void
LoadConfigFile()
{
    const int readBuffer_size = 300000;
    char *readBuffer = new char[readBuffer_size];
    readBuffer[0] = '\0';
    DWORD bytesRead = 0;        

    HANDLE configFile =
        CreateFile(
            configFilename,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL,
            NULL);
    bool readSuccess = ReadFile(configFile, readBuffer, readBuffer_size, &bytesRead, NULL);
    if (!readSuccess)
    {
        printf("Could not read from config file!\n");
    }

    CloseHandle(configFile);

    int currentCommandIndex = 0;
    char *readCharDest = LoadedConfigFile.Configs[currentCommandIndex].name;
    bool readingName = true;
    for (int i = 3; i < bytesRead; i++)
    {
        char currentChar = readBuffer[i];
        if (currentChar == '"')
        {
            if (readingName)
            {
                readCharDest = LoadedConfigFile.Configs[currentCommandIndex].name;
            }
            else
            {
                readCharDest = LoadedConfigFile.Configs[currentCommandIndex].command;
            }
        }
        else if (currentChar == ':')
        {
            readingName = !readingName;
        }
        else if (currentChar == '\r')
        {
            i++;
            currentCommandIndex++;
            LoadedConfigFile.configCount++;
            readingName = true;
        }
        else
        {
            char charWithNull[2];
            charWithNull[0] = currentChar;
            charWithNull[1] = '\0';
            strcat_s(readCharDest, 255, charWithNull);
        }
    }

    delete[] readBuffer;
}



int CALLBACK
WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    char      *cmdLine,
    int       cmdShow
)
{
    // Set up and display console for debugging.
    if (AllocConsole())
    {        
        // Set up handle to my standard out. The console needs to exist for the handle to return valid.
        My_Out = GetStdHandle(STD_OUTPUT_HANDLE);

        FILE *cout;
        freopen_s(&cout, "CONOUT$", "w", stdout);
        SetConsoleTitle(TITLE);
        SetConsoleTextAttribute(My_Out, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
        SetConsoleCtrlHandler(ConsoleCtrlHandler, true);
    }
    else
    {
        // @log error, could not initialize console
        return EXIT_FAILURE;
    }

    OutBuffer Buffer;
    std::streambuf *sb = std::cout.rdbuf(&Buffer);
    std::cout.rdbuf(sb);
    


    // Load and parse config file.
    LoadConfigFile();



    // Set up applicaion.
    WNDCLASSEX WindowClass    = { 0 };
    WindowClass.cbSize	      = sizeof(WNDCLASSEX);
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



    // Set up window.
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

    // @todo only show window on click or keyboard shortcut.
    //ShowWindow(MyWindow, nCmdShow);
    UpdateWindow(MyWindow);



    // Set up cleanner upper for the notification icon no matter what happens. (Except stopping debugging still leaves garbage in the tray.)
    SetUnhandledExceptionFilter(UnexpectedExitHandler);


    
    // Set up tray icon menu.
    IconMenu = CreatePopupMenu();
    AppendMenu(IconMenu, MF_STRING,    IconMenu_RunCancel, L"Run");
    AppendMenu(IconMenu, MF_SEPARATOR, 0,                  NULL);
    AppendMenu(IconMenu, MF_STRING,    IconMenu_Exit,      L"Exit");



    // Set up tray icon.
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
    StringCchCopy(IconData.szTip,       ARRAYSIZE(IconData.szTip),       TITLE);
    StringCchCopy(IconData.szInfoTitle, ARRAYSIZE(IconData.szInfoTitle), TITLE);
    StringCchCopy(IconData.szInfo,      ARRAYSIZE(IconData.szInfo),      TITLE);

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
            // @log unable to start, couldn't create shell icon.
            MessageBox(
                MyWindow,
                L"Call to Shell_NotifyIcon failed!",
                TITLE,
                NULL);

            return EXIT_FAILURE;
        }
    }



    // Set up global hook.
    //KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)KeyboardEvent, Instance, NULL);


    
    // Main message loop.
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

    if (My_Out != NULL || My_Out == INVALID_HANDLE_VALUE)
    {
        if(!CloseHandle(My_Out))
        {
            printf("Couldn't close My_Out!\n");
        }
    }

    if (isChildRunning)
    {
        TerminateChild();
    }
}

void
TerminateChild()
{    
    if (ChildProcInfo.hProcess != NULL)
    {
        if(TerminateProcess(ChildProcInfo.hProcess, 0))
        {
            isChildRunning = false;
        }
        else
        {
            // @log error terminating process
            printf("Couldn't terminate process!\n");
        }         


        if (Child_In_Read != NULL && Child_In_Read != INVALID_HANDLE_VALUE)
        {
            if(!CloseHandle(Child_In_Read)) printf("Couldn't close Child_In_Read!\n");
            else Child_In_Read = NULL;
        }

        if (Child_In_Write != NULL && Child_In_Write != INVALID_HANDLE_VALUE)
        {
            if(!CloseHandle(Child_In_Write)) printf("Couldn't close Child_In_Write!\n");
            else Child_In_Write = NULL;
        }

        if (Child_Out_Read != NULL && Child_Out_Read != INVALID_HANDLE_VALUE)
        {
            if(!CloseHandle(Child_Out_Read)) printf("Couldn't close Child_Out_Read!\n");
            else Child_Out_Read = NULL;
        }

        if (Child_Out_Write != NULL && Child_Out_Write != INVALID_HANDLE_VALUE)
        {
            if(!CloseHandle(Child_Out_Write)) printf("Couldn't close Child_Out_Write!\n");
            else Child_Out_Write = NULL;
        }
    }
}

void
RunCancel()
{
    if (isChildRunning)
    {
        printf("Child still running! Attempting to stop...\n");
        wasChildCancelled = true;
        TerminateChild();
        return;
    }


    printf("Running.\n");
    
    // Get clipboard data.
    // Try to open the clipboard.
    if (!OpenClipboard(MyWindow))
    {
        // On failure, give up;
        // @log err.
        printf("Couldn't open the clipboard!\n");
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
            goto textData_cleanup;
        }

        wchar_t *wideText = static_cast<wchar_t*>(GlobalLock(textDataHandle));
        if (wideText == NULL)
        {
            goto textData_cleanup;
        }

        char text[20000];
        size_t numCharsConverted = 0;
        wcstombs_s(&numCharsConverted, text, wideText, _TRUNCATE);


        // Start of child process creation.
        SECURITY_ATTRIBUTES secAttr;
        secAttr.nLength              = sizeof(secAttr);
        secAttr.bInheritHandle       = true;
        secAttr.lpSecurityDescriptor = NULL;

        // Create and initialize standard in pipe.
        {
            bool createPipeSuccess =
                CreatePipe(
                    &Child_In_Read,
                    &Child_In_Write,
                    &secAttr,
                    0);
            if (!createPipeSuccess)
            {
                // @logging log error.
                printf("Could not create standard in pipe!\n");
                goto textData_cleanup;
            }

            bool setPipeFlagSuccess = SetHandleInformation(Child_In_Write, HANDLE_FLAG_INHERIT, 0);
            if (!setPipeFlagSuccess)
            {
                // @logging log error.
                printf("Could not set standard in pipe information!\n");
                goto textData_cleanup;
            }
        }

        
        // Create and initialize standard out pipe.
        {
            bool createPipeSuccess =
                CreatePipe(
                    &Child_Out_Read,
                    &Child_Out_Write,
                    &secAttr,
                    0);
            if (!createPipeSuccess)
            {
                // @logging log error.
                printf("Could not create standard out pipe!\n");
                goto textData_cleanup;
            }

            bool setPipeFlagSuccess = SetHandleInformation(Child_Out_Read, HANDLE_FLAG_INHERIT, 0);
            if (!setPipeFlagSuccess)
            {
                // @logging log error.
                printf("Could not set standard out pipe information!\n");
                goto textData_cleanup;
            }
        }       

        // Write to the processes' standard in.
        {
            DWORD bytesWritten = 0;
            bool writeSuccess = WriteFile(Child_In_Write, text, strnlen_s(text, 20000), &bytesWritten, NULL);
            if (!writeSuccess)
            {
                // @logging log error.
                printf("Could not write to child's standard in!\n");
                goto textData_cleanup;
            }
        }
        
        wchar_t tempFileNameAndPath[MAX_PATH];
        {
            TCHAR tempPath[MAX_PATH];
            DWORD tempPathLength = GetTempPath(MAX_PATH, tempPath);
        
            // GetTempFileName CREATES A FILE IF IT SUCCEEDS.
            GetTempFileName(tempPath, TITLE, 0, tempFileNameAndPath);
            printf("Temp file path and name: %ls\n", tempFileNameAndPath);

            char openFileName[MAX_PATH];
            HANDLE tempFile =
                CreateFile(
                    tempFileNameAndPath,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING, 
                    FILE_ATTRIBUTE_NORMAL,
                    NULL); 
            
            DWORD bytesWritten = 0;
            bool writeSuccess = WriteFile(tempFile, text, strnlen_s(text, 20000), &bytesWritten, NULL);
            if (!writeSuccess)
            {
                // @logging log error.
                printf("Could not write to temp file!\n");
                goto textData_cleanup;
            }

            
            CloseHandle(tempFile);
        }
        


        // Build command string.
        const int commandLine_size = 4096;
        wchar_t commandLine[commandLine_size];
        commandLine[0] = '\0';
        //wchar_t commandLine[commandLine_size] = L"..\\Debug\\Echoer.exe ";

        // Add command as zeroith argument.
        wchar_t convertedCommand[255];
        size_t numConverted = 0;
        mbstowcs_s(&numConverted, convertedCommand, LoadedConfigFile.Configs[currentConfigIndex].command, _TRUNCATE);
        wcscat_s(commandLine, convertedCommand);
        wcscat_s(commandLine, L" ");

        // Add temp file path as the first argument.
        wcscat_s(commandLine, tempFileNameAndPath);
        wcscat_s(commandLine, L" ");

        // Add clipboard data for the remaining arguments.
        wcscat_s(commandLine, wideText);


  
        // Create the child process.
        {
            STARTUPINFO startupInfo = { 0 };
            startupInfo.cb         = sizeof(startupInfo);
            startupInfo.hStdInput  = Child_In_Read;
            startupInfo.hStdError  = Child_Out_Write;
            startupInfo.hStdOutput = Child_Out_Write;
            startupInfo.dwFlags    = STARTF_USESTDHANDLES;

            bool createProcessSuccess = CreateProcessW(
                NULL,
                commandLine,
                NULL,
                NULL,
                true,
                0,
                NULL,
                NULL,
                &startupInfo,
                &ChildProcInfo);        
            if (!createProcessSuccess)
            {
                printf("Could not start child process with command line: %ls", commandLine);
                goto textData_cleanup;
            }

            isChildRunning = true;
            ModifyMenu(IconMenu, IconMenu_RunCancel, MF_BYCOMMAND, IconMenu_RunCancel, L"Cancel");

            // newHandle is always 0x00000000 so I'm assuming I don't need to clean it up.
            HANDLE newHandle;
            RegisterWaitForSingleObject(&newHandle, ChildProcInfo.hProcess, LaunchedProcessExitedOrCancelled, NULL, INFINITE, WT_EXECUTEONLYONCE);
        }
        
    textData_cleanup:
        // GlobalUnlock makes the handle invalid - no need to call CloseHandle.
        GlobalUnlock(textDataHandle);
    }
    else
    {
        // Error or no data.
        goto clipboard_cleanup;
    }

clipboard_cleanup:
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
                RunCancel();
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
                    break;
                }

                case WM_RBUTTONDOWN:
                {
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
                            DestroyWindow(Window);
                            break;
                        }

                        case IconMenu_RunCancel:
                        {
                            RunCancel();
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

VOID CALLBACK
LaunchedProcessExitedOrCancelled(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    isChildRunning = false;
    ModifyMenu(IconMenu, IconMenu_RunCancel, MF_BYCOMMAND, IconMenu_RunCancel, L"Run");

    if (wasChildCancelled)
    {
        wasChildCancelled = false;
        printf("Child cancelled.\n");
        goto child_cleanup;
    }

    printf("Child exited.\n");
    // @todo is this big enough?
    const int pipeBuffer_size = 500000;
    char *pipeBuffer = new char[pipeBuffer_size];
            
    {
        // Read from processes' standard out.
        DWORD numBytesRead = 0;
        bool readSuccess = ReadFile(Child_Out_Read, pipeBuffer, pipeBuffer_size, &numBytesRead, NULL);
        if (!readSuccess)
        {
            // @logging log error.
            printf("Could not read from child's standard out!\n");
            goto pipeBuffer_cleanup;
        }

        // Write to my standard out.
        DWORD numBytesWritten = 0;
        bool writeSuccess = WriteFile(My_Out, pipeBuffer, numBytesRead, &numBytesWritten, NULL);
        if (!writeSuccess)
        {
            // @logging log error.
            printf("Could not write to my standard out!\n");
            goto pipeBuffer_cleanup;
        }

        // Write to global memory for clipboard.        
        HANDLE clipboardMemory = GlobalAlloc(GMEM_MOVEABLE, numBytesRead);
        if (clipboardMemory == NULL)
        {
            // @logging log error.
            printf("Couldn't allocate global memory! Size:%d\n", numBytesRead);
            goto pipeBuffer_cleanup;
        }

        HANDLE writeableClipboardMemory = GlobalLock(clipboardMemory);
        memcpy_s(writeableClipboardMemory, numBytesRead, pipeBuffer, numBytesRead);
        GlobalUnlock(writeableClipboardMemory);
        
        printf("\n\n\nAttempting to open clipboard...\n");
    
        if (!OpenClipboard(MyWindow))
        {
            // On failure, give up;
            // @log err.
            printf("Couldn't open the clipboard!\n");
            goto pipeBuffer_cleanup;
        }

        EmptyClipboard();
        SetClipboardData(CF_TEXT, clipboardMemory);        
        printf("Wrote to the clipboard!\n");
    }
    
    printf("Back to normal.\n");



clipboard_cleanup:
    CloseClipboard();
pipeBuffer_cleanup:    
    delete[] pipeBuffer;
child_cleanup:
    ChildProcInfo = { 0 };
}
