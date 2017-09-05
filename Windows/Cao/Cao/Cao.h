#pragma once
#include "resource.h"



namespace MainWindow
{
    typedef struct
    {
        LPCSTR windowTitle;
        HINSTANCE instance;
        LPCSTR windowClassName;
        WNDCLASS windowClass;
        HWND window;
        COLORREF transparentColor;
        int screenWidth;
        int screenHeight;
    } State;
}



void DrawRect(int x, int y, int width, int height, HDC deviceContext, HBRUSH brush);

int WINAPI wWinMain(HINSTANCE mainInstance, HINSTANCE, PWSTR cmdLine, int cmdShow);
