#pragma once

// @todo enums
const char Config_CONTROL = 1 << 0;
const char Config_ALT     = 1 << 1;
const char Config_SHIFT   = 1 << 2;

const char Config_STANDARD = 1 << 0;
const char Config_TEMPFILE = 1 << 1;
const char Config_ARGS      = 1 << 2;

const int name_size = 255;
const int command_size = MAX_PATH;
typedef struct Config
{
    char name[name_size];
    char command[command_size];
    char inputMode;
    char hotkeyMod;
    char hotkey;
} Config;

void
RegisterConfigChangeNotifer();

VOID CALLBACK
ConfigChanged(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

void
LoadConfigFile();

int
WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    char * cmdLine,
    int cmdShow);

void
CleanupStuff();

void
Cancel();

void
Run(Config *config);

LRESULT CALLBACK
WndProc(
    HWND Window,
    UINT message,
    WPARAM wParam,
    LPARAM lParam);

LONG WINAPI
UnexpectedExitHandler(PEXCEPTION_POINTERS pExceptionInfo);

BOOL WINAPI
ConsoleCtrlHandler(
    DWORD controlType);

VOID CALLBACK 
LaunchedProcessExitedOrCancelled(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired);

void
LoadConfigFile();

void
TerminateChild();

void
HandleIconMessage(LPARAM message);

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