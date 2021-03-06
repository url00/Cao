// Compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c /F 8000000 
#pragma comment(lib, "user32.lib")

#include <Windows.h>
#include <strsafe.h>
#include <string>
#include <fstream>
#include <iostream>
#include <assert.h>
#include <sstream>
#include <algorithm>

#include "Main.h"



// Global data.
static const wchar_t *TITLE = L"Cao";
static HWND MainWindow = NULL;
static HHOOK KeyboardHook;
static HANDLE My_Out = NULL;
static bool isWindowShowing = false;



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
static HANDLE childWait = NULL;


static const char *configFilenameA    =  "config.txt";
static const wchar_t *configFilenameW = L"config.txt";

static int Configs_count = 0;
static const int Configs_size = 200;
static Config Configs[Configs_size];

static HANDLE configChangeNotifier     = NULL;
static HANDLE configChangeNotifierWait = NULL;


static RAWINPUTDEVICE rawKeyboard = { 0 };
static char modState = 0;



void DisplayConfigFileError(int lineNum)
{
    // @todo give more precisce error position
    // @log err
    wchar_t errorMessage[255];
    errorMessage[0] = '\0';
    StringCchPrintf(errorMessage, 255, L"Invalid config file! Error line: %d", lineNum);

    MessageBox(MainWindow, errorMessage, TITLE, MB_OK);
    PostQuitMessage(0);
}


void
LoadConfigFile()
{
    printf("Loading config file.\n");

    memset(Configs, 0, sizeof(Configs));
    Configs_count = 0;

    using namespace std;
    
    ifstream file(configFilenameA);
    string rawLine;
    int lineNum = 0;
    while (getline(file, rawLine))
    {
        lineNum++;

        if (rawLine.length() < 5)
        {
            continue;
        }
        
        int commentPosition = rawLine.find('#');
        if (commentPosition != string::npos)
        {
            rawLine = rawLine.substr(0, commentPosition);
        }

        
        int commandPos = rawLine.find('~');
        if (commandPos == string::npos || commandPos + 1 >= rawLine.length() - 1)
        {
            continue;
        }


        std::string configPart  = rawLine.substr(0, commandPos);
        std::string commandPart = rawLine.substr(commandPos + 1, rawLine.length());


        int numDelimiters = count(configPart.begin(), configPart.end(), ',');
        int namePos = configPart.rfind(',');
        if (numDelimiters != 3)
        {
            continue;
        }


        
        Config *config = &Configs[Configs_count];

        stringstream configPart_stream;
        configPart_stream << configPart;
        
        string hotkeyMod;
        getline(configPart_stream, hotkeyMod, ',');
        if (hotkeyMod.find('c') != string::npos) config->hotkeyMod |= Config_CONTROL;
        if (hotkeyMod.find('a') != string::npos) config->hotkeyMod |= Config_ALT;
        if (hotkeyMod.find('s') != string::npos) config->hotkeyMod |= Config_SHIFT;
        if (config->hotkeyMod == 0)
        {
            DisplayConfigFileError(lineNum);
            return;
        }

        string hotkey;
        getline(configPart_stream, hotkey, ',');
        if (hotkey.length() != 1)
        {
            DisplayConfigFileError(lineNum);
            return;
        }
        config->hotkey = hotkey[0];

        string mode;
        getline(configPart_stream, mode, ',');
        if (mode.find('s') != string::npos) config->inputMode |= Config_STANDARD;
        if (mode.find('t') != string::npos) config->inputMode |= Config_TEMPFILE;
        if (mode.find('a') != string::npos) config->inputMode |= Config_ARGS;
        if (config->inputMode == 0)
        {
            DisplayConfigFileError(lineNum);
            return;
        }

        string name;
        getline(configPart_stream, name);
        strcpy_s(config->name, name.c_str());



        if (commandPart.length() <= 0)
        {
            DisplayConfigFileError(lineNum);
            return;
        }
        strcpy_s(config->command, commandPart.c_str());


        Configs_count++;
    }

    
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
    WindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
    WindowClass.lpszMenuName  = NULL;
    WindowClass.lpszClassName = TITLE;
    WindowClass.hIconSm       = LoadIcon(WindowClass.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&WindowClass))
    {
        MessageBox(NULL,
            L"Call to RegisterClassEx failed!",
            TITLE,
            NULL);

        return EXIT_FAILURE;
    }



    // Set up window.
    // @todo get current screen size to put window in center
    MainWindow =
        CreateWindow(
            WindowClass.lpszClassName,
            TITLE,
            WS_OVERLAPPEDWINDOW,
            490, 500,
            1000, 400,
            NULL,
            NULL,
            Instance,
            NULL
        );

    if (!MainWindow)
    {
        MessageBox(
            NULL,
            L"Call to CreateWindow failed!",
            TITLE,
            NULL);

        return EXIT_FAILURE;
    }



    //ShowWindow(MainWindow, cmdShow);
    UpdateWindow(MainWindow);



    // Set up cleanner upper for the notification icon no matter what happens. (Except stopping debugging still leaves garbage in the tray.)
    SetUnhandledExceptionFilter(UnexpectedExitHandler);


    
    // Set up tray icon menu.
    IconMenu = CreatePopupMenu();
    AppendMenu(IconMenu, MF_STRING,    IconMenu_RunCancel, L"Run");
    AppendMenu(IconMenu, MF_SEPARATOR, 0,                  NULL);
    AppendMenu(IconMenu, MF_STRING,    IconMenu_Exit,      L"Exit");

    // Set up tray icon.
    IconData.cbSize           = sizeof(NOTIFYICONDATA);
    IconData.hWnd             = MainWindow;
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
    IconData.hIcon            = LoadIcon(NULL, IDI_ASTERISK);
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
                MainWindow,
                L"Call to Shell_NotifyIcon failed!",
                TITLE,
                NULL);

            return EXIT_FAILURE;
        }
    }


    
    // Load and parse config file.
    LoadConfigFile();
    RegisterConfigChangeNotifer();



    // Set up raw input.
    rawKeyboard.dwFlags     = RIDEV_NOLEGACY | RIDEV_INPUTSINK;
    rawKeyboard.usUsagePage = 1;
    rawKeyboard.usUsage     = 6;
    rawKeyboard.hwndTarget  = MainWindow;
    RegisterRawInputDevices(&rawKeyboard, 1, sizeof(rawKeyboard));



    
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
RegisterConfigChangeNotifer()
{
    if (configChangeNotifier != NULL || configChangeNotifier != INVALID_HANDLE_VALUE)
    {
        FindCloseChangeNotification(configChangeNotifier);
        configChangeNotifier = NULL;
    }

    if (configChangeNotifierWait != NULL || configChangeNotifierWait != INVALID_HANDLE_VALUE)
    {
        UnregisterWait(configChangeNotifierWait);
        configChangeNotifierWait = NULL;
    }

    configChangeNotifier =
        FindFirstChangeNotification(L".", false, FILE_NOTIFY_CHANGE_LAST_WRITE);
    if (configChangeNotifier == INVALID_HANDLE_VALUE)
    {
        // @log err
        printf("Could not initialize config file watcher!\n");
        return;
    }
        
    bool success =
        RegisterWaitForSingleObject(
            &configChangeNotifierWait,
            configChangeNotifier,
            ConfigChanged,
            NULL,
            INFINITE,
            WT_EXECUTEONLYONCE);
    if (!success)
    {
        // @log err
        printf("Could not register wait for config file watcher!\n");
        return;
    }
}



VOID CALLBACK
ConfigChanged(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{     
    HANDLE currentDir =
        CreateFile(
            L".",
            FILE_LIST_DIRECTORY,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);
    if (currentDir == INVALID_HANDLE_VALUE)
    {
        // @log err
        printf("Could not create handle for current directoy!\n");
    }
    
    DWORD bytesWritten;
    FILE_NOTIFY_INFORMATION infos[4096];
    memset(infos, 0, sizeof(infos));
    
    bool success =
        ReadDirectoryChangesW(
            currentDir,
            &infos,
            sizeof(infos),
            false,
            FILE_NOTIFY_CHANGE_LAST_WRITE,
            &bytesWritten,
            NULL,
            NULL);

    int infos_count = bytesWritten / sizeof(FILE_NOTIFY_INFORMATION);
    for (int infos_i = 0; infos_i < infos_count; infos_i++)
    {
        FILE_NOTIFY_INFORMATION *info = &infos[infos_i];

        if (!(info->Action & FILE_ACTION_MODIFIED)) continue;
        
        // @todo replace hardcoded filename length
        if (info->FileNameLength != 20) continue;



        wchar_t safeFilename[MAX_PATH];
        memcpy_s(safeFilename, MAX_PATH, info->FileName, info->FileNameLength);
        safeFilename[info->FileNameLength / 2] = '\0';

        // @todo replace hardcoded filename
        if (wcscmp(configFilenameW, safeFilename) == 0)
        {
            LoadConfigFile();
            break;
        }
    }

    CloseHandle(currentDir);

    RegisterConfigChangeNotifer();
}



void
CleanupStuff()
{
    // @todo try to delete temp file(s).

    UnhookWindowsHookEx(KeyboardHook);
    Shell_NotifyIcon(NIM_DELETE, &IconData);

    if (My_Out != NULL && My_Out != INVALID_HANDLE_VALUE)
    {
        if(!CloseHandle(My_Out))
        {
            printf("Couldn't close My_Out!\n");
        }

        My_Out = NULL;
    }

    if (isChildRunning)
    {
        TerminateChild();
    }

    if (configChangeNotifier != NULL && configChangeNotifier != INVALID_HANDLE_VALUE)
    {
        bool success = FindCloseChangeNotification(configChangeNotifier);
        if (!success)
        {
            // @log err
            printf("Could not close folder watcher handle!");
        }
    }
}

void
TerminateChild()
{    
    if (ChildProcInfo.hProcess != NULL && ChildProcInfo.hProcess != INVALID_HANDLE_VALUE)
    {
        // TerminateProcess closes the handle.
        if(TerminateProcess(ChildProcInfo.hProcess, 0))
        {
            isChildRunning = false;
        }
        else
        {
            // @log error terminating process
            printf("Couldn't terminate process!\n");

            if(!CloseHandle(ChildProcInfo.hProcess)) printf("Couldn't close handle to child process!\n");

            ChildProcInfo = { 0 };
        } 


        if (Child_In_Read != NULL && Child_In_Read != INVALID_HANDLE_VALUE)
        {
            if(!CloseHandle(Child_In_Read))
            {
                printf("Couldn't close Child_In_Read!\n");
            }

            Child_In_Read = NULL;
        }

        if (Child_In_Write != NULL && Child_In_Write != INVALID_HANDLE_VALUE)
        {
            if(!CloseHandle(Child_In_Write))
            {
                printf("Couldn't close Child_In_Write!\n");
            }

            Child_In_Write = NULL;
        }

        if (Child_Out_Read != NULL && Child_Out_Read != INVALID_HANDLE_VALUE)
        {
            if(!CloseHandle(Child_Out_Read))
            {
                printf("Couldn't close Child_Out_Read!\n");
            }

            Child_Out_Read = NULL;
        }

        if (Child_Out_Write != NULL && Child_Out_Write != INVALID_HANDLE_VALUE)
        {
            if(!CloseHandle(Child_Out_Write))
            {
                printf("Couldn't close Child_Out_Write!\n");
            }

            Child_Out_Write = NULL;
        }
    }
}

void
Cancel()
{
    if (!isChildRunning)
    {
        printf("Nothing to cancel.\n");
        return;
    }

    
    printf("Attempting to stop...\n");
    wasChildCancelled = true;
    TerminateChild();
    return;
}

void
Run(Config *config)
{
    if (isChildRunning)
    {
        return;
    }

    printf("Running command \"%s\".\n", config->name);
    
    // Get clipboard data.
    if (!OpenClipboard(MainWindow))
    {
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
    do
    {
        switch (currentFormat)
        {
            case CF_UNICODETEXT:
            {
                isText = true;
                break;
            }

            default:
            {
                currentFormat = EnumClipboardFormats(currentFormat);
                break;
            }
        }

    } while (currentFormat != firstFormat && !isText);
    


    if (isText)
    {
        HGLOBAL textDataHandle = GetClipboardData(CF_UNICODETEXT);
        if (textDataHandle == NULL)
        {
            // @log err
            goto textData_cleanup;
        }

        wchar_t *wideText = static_cast<wchar_t*>(GlobalLock(textDataHandle));
        if (wideText == NULL)
        {
            // @log err
            goto textData_cleanup;
        }

        const int text_size = 200000;
        int text_numBytes = 0;
        char text[text_size];
        {
            text_numBytes =
                WideCharToMultiByte(
                    CP_UTF8,
                    NULL,      // Must be set to NULL for CP_UTF8?
                    wideText,
                    -1,        // Null terminated, so no size needed.
                    text,
                    text_size,
                    NULL,      // Must be set to NULL for CP_UTF8.
                    NULL);     // Must be set to NULL for CP_UTF8.

            text[text_numBytes] = '\0';
        }


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
                    text_size); // If this buffer size "suggestion" isn't good enough, try CreateNamedPipe.
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
                    text_size);
            if (!createPipeSuccess)
            {
                // @logging log error.
                printf("Could not create standard out pipe!\n");
                goto textData_cleanup;
            }

            bool setPipeFlagSuccess =
                SetHandleInformation(
                    Child_Out_Read,
                    HANDLE_FLAG_INHERIT,
                    0);
            if (!setPipeFlagSuccess)
            {
                // @logging log error.
                printf("Could not set standard out pipe information!\n");
                goto textData_cleanup;
            }
        }
        
        const int commandLine_size = 200000;
        wchar_t commandLine[commandLine_size];
        commandLine[0] = '\0';
        // Add command as zeroith argument.
        wchar_t convertedCommand[command_size];
        size_t numConverted = 0;
        mbstowcs_s(&numConverted, convertedCommand, config->command, _TRUNCATE);
        wcscat_s(commandLine, L"cmd /C \"");
        wcscat_s(commandLine, convertedCommand);
        
        if (config->inputMode & Config_TEMPFILE)
        {
            wcscat_s(commandLine, L" ");

            wchar_t tempFileNameAndPath[MAX_PATH];
            TCHAR tempPath[MAX_PATH];
            GetTempPath(MAX_PATH, tempPath);
        
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
            
            DWORD tempBytesWritten = 0;
            bool writeSuccess =
                WriteFile(
                    tempFile,
                    text,
                    text_numBytes,
                    &tempBytesWritten,
                    NULL);
            if (!writeSuccess)
            {
                // @logging log error.
                printf("Could not write to temp file!\n");
                goto textData_cleanup;
            }

            
            CloseHandle(tempFile);
            
            // Add temp file path as the first argument.
            wcscat_s(commandLine, tempFileNameAndPath);
            wcscat_s(commandLine, L" ");
        }        

        if (config->inputMode & Config_ARGS)
        {
            wcscat_s(commandLine, L" ");

            // Add clipboard data for the remaining arguments.
            wcscat_s(commandLine, wideText);
        }
        
        wcscat_s(commandLine, L"\"");

        printf("commandLine: %ls\n", commandLine);


  
        // Create the child process.
        {
            STARTUPINFO startupInfo = { 0 };
            startupInfo.cb         = sizeof(startupInfo);
            startupInfo.hStdInput  = Child_In_Read;
            startupInfo.hStdError  = Child_Out_Write;
            startupInfo.hStdOutput = Child_Out_Write;
            startupInfo.dwFlags    = STARTF_USESTDHANDLES;

            bool createProcessSuccess = CreateProcess(
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

            RegisterWaitForSingleObject(&childWait, ChildProcInfo.hProcess, LaunchedProcessExitedOrCancelled, NULL, INFINITE, WT_EXECUTEONLYONCE);

            

            // Need to write to standard in in chuncks...?
            // Why does pseudo-buffering these writes work with some commands?
            // This shouldn't work here, yet it does. Why?
            if (config->inputMode & Config_STANDARD)
            {
                int bytesLeftToWrite = text_numBytes;
                while (bytesLeftToWrite > 128)
                {
                    int startingOffset = text_numBytes - bytesLeftToWrite;
                    DWORD inBytesWritten = 0;
                    bool writeSuccess =
                        WriteFile(
                            Child_In_Write,
                            text + startingOffset,
                            128,
                            &inBytesWritten,
                            NULL);
                    if (!writeSuccess)
                    {
                        // @logging log error.
                        printf("Could not write to child's standard in!\n");
                        goto textData_cleanup;
                    }

                    bytesLeftToWrite -= inBytesWritten;
                }

                assert(bytesLeftToWrite >= 0);

                if (bytesLeftToWrite > 0)
                {                    
                    int startingOffset = text_numBytes - bytesLeftToWrite;
                    DWORD inBytesWritten = 0;
                    bool writeSuccess =
                        WriteFile(
                            Child_In_Write,
                            text + startingOffset,
                            bytesLeftToWrite,
                            &inBytesWritten,
                            NULL);
                    if (!writeSuccess)
                    {
                        // @logging log error.
                        printf("Could not write to child's standard in!\n");
                        goto textData_cleanup;
                    }
                }
            }


            CloseHandle(Child_In_Write);
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

void
HandleIconMessage(LPARAM message)
{
    switch (message)
    {
        case WM_LBUTTONDBLCLK:
        {
            if (isWindowShowing)
            {
                isWindowShowing = false;
                ShowWindow(MainWindow, SW_HIDE);
            }
            else
            {
                isWindowShowing = true;
                ShowWindow(MainWindow, SW_RESTORE);
            }
            break;
        }

        case WM_RBUTTONDOWN:
        {
            POINT CursorPoint;
            GetCursorPos(&CursorPoint);

            SetForegroundWindow(MainWindow);

            UINT clicked =
                TrackPopupMenu(
                    IconMenu,
                    TPM_RETURNCMD | TPM_NONOTIFY,
                    CursorPoint.x,
                    CursorPoint.y,
                    0,
                    MainWindow,
                    NULL);
            switch (clicked)
            {
                case IconMenu_Exit:
                {
                    DestroyWindow(MainWindow);
                    break;
                }

                case IconMenu_RunCancel:
                {
                    if (isChildRunning)
                    {
                        Cancel();
                    }
                    else
                    {
                        Run(&Configs[0]);
                    }
                    break;
                }
            }

            break;
        }
    }
}

LRESULT CALLBACK
WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INPUT:
        {
            HRAWINPUT rawInputParam = (HRAWINPUT)lParam;
            UINT size = 0;

            {
                int result =
                    GetRawInputData(
                        rawInputParam,
                        RID_INPUT,
                        NULL,
                        &size,
                        sizeof(RAWINPUTHEADER));
                if (result == -1)
                {
                    break;
                }
            }

            // @todo is this really needed? it's a lot of allocs...
            // replace with a big enough static buffer?
            char *buffer = new char[size];
            if (buffer == NULL)
            {
                goto buffer_cleanup;
            }
            
            {
                int result =
                    GetRawInputData(
                        rawInputParam,
                        RID_INPUT,
                        buffer,
                        &size,
                        sizeof(RAWINPUTHEADER));
                if (result != size)
                {
                    goto buffer_cleanup;
                }
            }



            PRAWINPUT input = (PRAWINPUT)buffer;
            int event = 0;

            /*
            printf("Keyboard:\tmake: %04x flags: %04x reserved: %04x extra: %08x msg: %04x, vk: %04x\n",
                   input->data.keyboard.MakeCode,
                   input->data.keyboard.Flags,
                   input->data.keyboard.Reserved,
                   input->data.keyboard.ExtraInformation,
                   input->data.keyboard.Message,
                   input->data.keyboard.VKey);
            */
                                    
            event = input->data.keyboard.Message;

            // Build a comparable key name from the keyboard's scan code.
            // @todo clean up
            unsigned char key = MapVirtualKeyEx(input->data.keyboard.MakeCode, MAPVK_VSC_TO_VK_EX, NULL);
            wchar_t wideKeyName[255];
            wideKeyName[0] = '\0';
            GetKeyNameText(input->data.keyboard.MakeCode << 16, wideKeyName, 255);
            size_t numBytesConv = 0;
            char keyName[255];
            wcstombs_s(&numBytesConv, keyName, 255, wideKeyName, 255);
            char keyLetter = -1;
            const int sizeOfSingleWideChar = 2;
            if (numBytesConv == sizeOfSingleWideChar)
            {
                keyLetter = tolower(keyName[0]);
            }


            if (event == WM_KEYDOWN || event == WM_SYSKEYDOWN)
            {

                if (key == VK_LCONTROL || key == VK_RCONTROL)
                {
                    modState |= Config_CONTROL;
                }
                else if (key == VK_LSHIFT || key == VK_RSHIFT)
                {
                    modState |= Config_SHIFT;
                }
                else if (key == VK_LMENU || key == VK_RMENU || key == VK_MENU)
                {
                    modState |= Config_ALT;
                }

                // @todo don't hard code the cancel command.
                if (modState == (Config_CONTROL | Config_SHIFT | Config_ALT) &&
                    keyLetter == 'c')
                {
                    Cancel();
                }

                for (int Configs_i = 0; Configs_i < Configs_count; Configs_i++)
                {
                    Config *currentConfig = &Configs[Configs_i];
                    
                    if (modState == currentConfig->hotkeyMod &&
                        currentConfig->hotkey == keyLetter)
                    {
                        Run(currentConfig);
                    }
                }

            }
            else if (event == WM_KEYUP || event == WM_SYSKEYUP)
            {       
                if (key == VK_LCONTROL || key == VK_RCONTROL)
                {
                    modState &= ~Config_CONTROL;
                }
                else if (key == VK_LSHIFT || key == VK_RSHIFT)
                {
                    modState &= ~Config_SHIFT;
                }
                else if (key == VK_LMENU || key == VK_RMENU || key == VK_MENU)
                {
                    modState &= ~Config_ALT;
                };
            }



        buffer_cleanup:
            delete[] buffer;
            buffer = NULL;
            input  = NULL;



            break;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT paintStruct;
            HDC context = BeginPaint(MainWindow, &paintStruct);

            wchar_t *message = L"Hi.";
            TextOut(context, 5, 5, message, wcslen(message));

            EndPaint(Window, &paintStruct);
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
            HandleIconMessage(lParam);
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
    if (ChildProcInfo.hProcess != NULL && ChildProcInfo.hProcess != INVALID_HANDLE_VALUE)
    {
        CloseHandle(ChildProcInfo.hProcess);
        ChildProcInfo.hProcess = NULL;
    }

    if (childWait != NULL && childWait != INVALID_HANDLE_VALUE)
    {
        UnregisterWait(childWait);
        childWait = NULL;
    }


    isChildRunning = false;
    ModifyMenu(IconMenu, IconMenu_RunCancel, MF_BYCOMMAND, IconMenu_RunCancel, L"Run");

    if (wasChildCancelled)
    {
        wasChildCancelled = false;
        printf("Child cancelled.\n");
        goto child_cleanup;
    }

    printf("Child exited.\n");

    // @todo delete temp file.

    const int pipeBuffer_size = 200000;
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
    
        if (!OpenClipboard(MainWindow))
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
