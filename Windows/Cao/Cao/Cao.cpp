#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "Cao.h"



static MainWindow_StateDef MainWindow_State;

bool MainWindow_Init(HINSTANCE instance, WNDPROC messageCallbackHandler, const int screenWidth, const int screenHeight, const int commandShow)
{
    MainWindow_State = { };
    MainWindow_State.transparentColor = RGB(255, 0, 255);
    MainWindow_State.crossState.screenWidth = screenWidth;
    MainWindow_State.crossState.screenHeight = screenHeight;
    MainWindow_State.currentCommandShow = commandShow;
    MainWindow_State.instance = instance;



    MainWindow_State.windowTitle = "Cao Drawing Surface";
    MainWindow_State.windowClass.lpfnWndProc = messageCallbackHandler;
    MainWindow_State.windowClass.hInstance = instance;
    MainWindow_State.windowClassName = "Cao Drawing Surface";
    MainWindow_State.windowClass.lpszClassName = MainWindow_State.windowClassName;
    RegisterClass(&MainWindow_State.windowClass);

    MainWindow_State.window = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, MainWindow_State.windowClassName, MainWindow_State.windowTitle, WS_OVERLAPPEDWINDOW, 0, 0, screenWidth, screenHeight, NULL, NULL, MainWindow_State.instance, NULL);
    if (MainWindow_State.window == NULL)
    {
        return false;
    }

    // Make the window click-throughable.
    {
        auto exStyle = GetWindowLong(MainWindow_State.window, GWL_EXSTYLE);
        exStyle |= WS_EX_TRANSPARENT;
        SetWindowLong(MainWindow_State.window, GWL_EXSTYLE, exStyle);
    }



    // Remove all bars and borders.
    {
        LONG lStyle = GetWindowLong(MainWindow_State.window, GWL_STYLE);
        lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
        SetWindowLong(MainWindow_State.window, GWL_STYLE, lStyle);
    }

    {
        LONG lExStyle = GetWindowLong(MainWindow_State.window, GWL_EXSTYLE);
        lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
        SetWindowLong(MainWindow_State.window, GWL_EXSTYLE, lExStyle);
    }

    // Ensure the window is moved to the top left.
    SetWindowPos(MainWindow_State.window, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

    // Enable window transparency.
    if (!SetLayeredWindowAttributes(MainWindow_State.window, MainWindow_State.transparentColor, 0, LWA_COLORKEY))
    {
        return false;
    }
}
    
LRESULT CALLBACK MainWindow_MessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            MainWindow_State.currentDeviceContext = BeginPaint(MainWindow_State.window, &ps);                   
            SelectObject(MainWindow_State.currentDeviceContext, GetStockObject(GRAY_BRUSH)); 



            HBRUSH transparentBrush = CreateSolidBrush(MainWindow_State.transparentColor);
            if (transparentBrush == NULL)
            {
                goto transparentBrush_clean;
            }
            FillRect(MainWindow_State.currentDeviceContext, &ps.rcPaint, transparentBrush);



            if (MainWindow_State.crossState.shouldDrawTest)
            {
                HBRUSH yellowBrush = CreateSolidBrush(RGB(255, 255, 0));
                if (yellowBrush == NULL)
                {
                    goto yellowBrush_clean;
                }
                MainWindow_State.currentBrush = yellowBrush;
                MainWindow_DrawTest(MainWindow_State.crossState);

                yellowBrush_clean:
                DeleteObject(yellowBrush);
                MainWindow_State.currentBrush = NULL;
            }


            
            transparentBrush_clean:
            DeleteObject(transparentBrush);
            EndPaint(MainWindow_State.window, &ps);

            deviceContext_clean:
            ReleaseDC(MainWindow_State.window, MainWindow_State.currentDeviceContext);
            MainWindow_State.currentDeviceContext = NULL;
            return 0;
        }
    }

    return DefWindowProc(window, message, wparam, lparam);
}

void MainWindow_DrawTestRect(int x, int y, int width, int height)
{    
    RECT testRect;
    testRect.left   = x;
    testRect.top    = y;
    testRect.right  = x + width;
    testRect.bottom = y + height;
    FillRect(MainWindow_State.currentDeviceContext, &testRect, MainWindow_State.currentBrush);
}

void MainWindow_Show()
{
    ShowWindow(MainWindow_State.window, MainWindow_State.currentCommandShow);
}



int WINAPI wWinMain(HINSTANCE wWinMain_instance, HINSTANCE, PWSTR commandLine, int commandShow)
{
    
    {
        const auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const auto screenHeight = GetSystemMetrics(SM_CYSCREEN);
        if (!MainWindow_Init(wWinMain_instance, MainWindow_MessageHandler, screenWidth, screenHeight, commandShow))
        {
            return 1;
        }
    }

    MainWindow_Show();
    MainWindow_State.crossState.shouldDrawTest = true;


    
    MSG message = { };
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }



    return 0;
}
