#include "stdafx.h"
#include "Cao.h"
#include "..\..\..\Cross\Cross.h"



namespace MainWindow
{
    static State state;

    bool Init(HINSTANCE instance, WNDPROC messageCallbackHandler, const int screenWidth, const int screenHeight)
    {
        state = { };
        state.transparentColor = RGB(255, 0, 255);
        state.screenWidth = screenWidth;
        state.screenHeight = screenHeight;



        state.windowTitle = "Cao Drawing Surface";
        state.instance = instance;
        state.windowClass.lpfnWndProc = messageCallbackHandler;
        state.windowClass.hInstance = instance;
        state.windowClassName = "Cao Drawing Surface";
        state.windowClass.lpszClassName = state.windowClassName;
        RegisterClass(&state.windowClass);

        state.window = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, state.windowClassName, state.windowTitle, WS_OVERLAPPEDWINDOW, 0, 0, screenWidth, screenHeight, NULL, NULL, state.instance, NULL);
        if (state.window == NULL)
        {
            return false;
        }



        // Remove all bars and borders.
        {
            LONG lStyle = GetWindowLong(state.window, GWL_STYLE);
            lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
            SetWindowLong(state.window, GWL_STYLE, lStyle);
        }

        {
            LONG lExStyle = GetWindowLong(state.window, GWL_EXSTYLE);
            lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
            SetWindowLong(state.window, GWL_EXSTYLE, lExStyle);
        }

        // Ensure the window is moved to the top left.
        SetWindowPos(state.window, NULL, 0,0,0,0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER);

        // Enable window transparency.
        if (!SetLayeredWindowAttributes(state.window, state.transparentColor, 0, LWA_COLORKEY))
        {
            return false;
        }
    }


        
    LRESULT CALLBACK MessageHandler(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
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
                HDC deviceContext = BeginPaint(state.window, &ps);
            
                SelectObject(deviceContext, GetStockObject(GRAY_BRUSH)); 



                HBRUSH transparentBrush = CreateSolidBrush(state.transparentColor);
                if (transparentBrush == NULL)
                {
                    goto transparentBrush_clean;
                }

                FillRect(deviceContext, &ps.rcPaint, transparentBrush);
                DeleteObject(transparentBrush);



                TCHAR szMessage[] = "Paint Beginner";
                UINT nLen = _tcslen(szMessage);
                TextOut(deviceContext, 100, 325, szMessage, nLen); 



                HBRUSH yellowBrush = CreateSolidBrush(RGB(255, 255, 0));
                if (yellowBrush == NULL)
                {
                    goto yellowBrush_clean;
                }
                const int rectSize = 100;
                DrawRect(0, 0, rectSize, rectSize, deviceContext, yellowBrush);
                DrawRect(state.screenWidth - rectSize, 0, 100, 100, deviceContext, yellowBrush);
                DrawRect(state.screenWidth - rectSize, state.screenHeight - rectSize, 100, 100, deviceContext, yellowBrush);
                DrawRect(0, state.screenHeight - rectSize, rectSize, rectSize, deviceContext, yellowBrush);
                DeleteObject(yellowBrush);

                Ellipse(deviceContext, 100, 100, 200, 300);


                
                yellowBrush_clean:
                transparentBrush_clean:
                EndPaint(state.window, &ps);

                deviceContext_clean:
                ReleaseDC(state.window, deviceContext);
                return 0;
            }
        }

        return DefWindowProc(window, message, wparam, lparam);
    }
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



int WINAPI wWinMain(HINSTANCE mainWindowInstance, HINSTANCE, PWSTR commandLine, int commandShow)
{
    
    {
        const auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
        const auto screenHeight = GetSystemMetrics(SM_CYSCREEN);
        if (!MainWindow::Init(mainWindowInstance, MainWindow::MessageHandler, screenWidth, screenHeight))
        {
            return 1;
        }
    }

    ShowWindow(MainWindow::state.window, commandShow);



    MSG message = { };
    while (GetMessage(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }



    return 0;
}





