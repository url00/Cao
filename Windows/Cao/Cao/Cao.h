#pragma once
#include "resource.h"



int WINAPI wWinMain(HINSTANCE mainInstance, HINSTANCE, PWSTR cmdLine, int cmdShow);

LRESULT CALLBACK mainWindow_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);