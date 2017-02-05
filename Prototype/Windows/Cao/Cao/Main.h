#pragma once

void
CleanupStuff();

void
CleanupProcessStuff();

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
LaunchedProcessExited(
    PVOID lpParameter,
    BOOLEAN TimerOrWaitFired
);

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
