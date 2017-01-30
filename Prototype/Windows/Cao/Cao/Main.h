#pragma once

void
CleanupStuff();

void
Run();

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
