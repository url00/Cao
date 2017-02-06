#pragma once

int
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, char * cmdLine, int cmdShow);

void
CleanupStuff();

void
RunCancel();

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

static const int name_size = 255;
static const int command_size = MAX_PATH;
typedef struct Config
{
    char name[name_size];
    char command[command_size];
} Config;