#pragma once
#include "resource.h"
#include "../../../Cross/Cross.h"



typedef struct
{
    LPCSTR windowTitle;
    HINSTANCE instance;
    LPCSTR windowClassName;
    WNDCLASS windowClass;
    HWND window;
    COLORREF transparentColor;
    HBRUSH currentBrush;
    HDC currentDeviceContext;
    int currentCommandShow;
    MainWindow_CrossStateDef crossState;
} MainWindow_StateDef;

int WINAPI wWinMain(HINSTANCE mainInstance, HINSTANCE, PWSTR cmdLine, int cmdShow);
