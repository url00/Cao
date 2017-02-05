#pragma once

int
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, char * cmdLine, int cmdShow);

void
CleanupStuff();

void
RunCancel();

LRESULT CALLBACK
KeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK
WndProc(HWND Window, UINT message, WPARAM wParam, LPARAM lParam);

LONG WINAPI
UnexpectedExitHandler(PEXCEPTION_POINTERS pExceptionInfo);

BOOL WINAPI
ConsoleCtrlHandler(
    DWORD controlType
);

VOID CALLBACK 
LaunchedProcessExitedOrCancelled(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired
);

void
LoadConfigFile();

void
TerminateChild();

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

typedef struct Config
{
    wchar_t name[255];
    wchar_t command[MAX_PATH];
} Config;

#define ConfigFil_Configs_size 200
typedef struct ConfigFile
{
    int configCount;
    Config Configs[ConfigFil_Configs_size];
} ConfigFile;