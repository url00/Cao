#include "stdafx.h"
#include "Cao.h"
#include "..\..\..\Cross\Cross.h"



static const LPCSTR mainWindow_className = "Sample Window Class";
static const auto transparentRGB = RGB(255, 0, 255);


static int screenWidth;
static int screenHeight;



int WINAPI wWinMain(HINSTANCE mainWindow_instance, HINSTANCE, PWSTR cmdLine, int cmdShow)
{
    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);



    WNDCLASS mainWindow_class = { };
    mainWindow_class.lpfnWndProc   = mainWindow_proc;
    mainWindow_class.hInstance     = mainWindow_instance;
    mainWindow_class.lpszClassName = mainWindow_className;
    RegisterClass(&mainWindow_class);



    HWND mainWindow =
        CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_LAYERED,
            mainWindow_className,
            "Main Window",
            WS_OVERLAPPEDWINDOW,
            0,
            0,
            screenWidth,
            screenHeight,
            NULL,
            NULL,
            mainWindow_instance,
            NULL
        );
    if (mainWindow == NULL)
    {
        return 0;
    }



    // Remove all bars and borders.
    LONG lStyle = GetWindowLong(mainWindow, GWL_STYLE);
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
    SetWindowLong(mainWindow, GWL_STYLE, lStyle);

    LONG lExStyle = GetWindowLong(mainWindow, GWL_EXSTYLE);
    lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
    SetWindowLong(mainWindow, GWL_EXSTYLE, lExStyle);

    SetWindowPos(mainWindow, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);



    // Enable window transparency.
    {
        bool didCallFail =
            !SetLayeredWindowAttributes(
                mainWindow,
                transparentRGB,
                0,
                LWA_COLORKEY);
        if (didCallFail)
        {
            // Error. #log #err #todo
        }
    }



    ShowWindow(mainWindow, cmdShow);
    


    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }



    return 0;
}



void DrawRect(int x, int y, int width, int height, HDC deviceContext, HBRUSH brush)
{    
    RECT testRect;
    testRect.left   = x;
    testRect.top    = y;
    testRect.right  = x + width;
    testRect.bottom = y + height;
    FillRect(deviceContext, &testRect, brush);
}



LRESULT CALLBACK mainWindow_proc(HWND proc_window, UINT proc_message, WPARAM proc_wprarm, LPARAM proc_lparam)
{
    switch (proc_message)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC deviceContext = BeginPaint(proc_window, &ps);
            
            SelectObject(deviceContext, GetStockObject(GRAY_BRUSH)); 



            HBRUSH transparentBrush = CreateSolidBrush(transparentRGB);
            if (transparentBrush == NULL)
            {
                // Error. #todo #log #err
                goto ps_clean;
            }

            FillRect(deviceContext, &ps.rcPaint, transparentBrush);
            DeleteObject(transparentBrush);



            TCHAR szMessage[] = "Paint Beginner";
            UINT nLen = _tcslen(szMessage);
            TextOut(deviceContext, 100, 325, szMessage, nLen); 



            HBRUSH yellowBrush = CreateSolidBrush(RGB(255, 255, 0));
            if (yellowBrush == NULL)
            {
                goto ps_clean;
            }
            const int rectSize = 100;
            DrawRect(0, 0, rectSize, rectSize, deviceContext, yellowBrush);
            DrawRect(screenWidth - rectSize, 0, 100, 100, deviceContext, yellowBrush);
            DrawRect(screenWidth - rectSize, screenHeight - rectSize, 100, 100, deviceContext, yellowBrush);
            DrawRect(0, screenHeight - rectSize, rectSize, rectSize, deviceContext, yellowBrush);
            DeleteObject(yellowBrush);




            Ellipse(deviceContext, 100, 100, 200, 300);
                
            ps_clean:
            EndPaint(proc_window, &ps);

            hdc_clean:
            ReleaseDC(proc_window, deviceContext);
            return 0;
        }
    }

    return DefWindowProc(proc_window, proc_message, proc_wprarm, proc_lparam);
}